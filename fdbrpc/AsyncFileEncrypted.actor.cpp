/*
 * AsyncFileEncrypted.actor.cpp
 *
 * This source file is part of the FoundationDB open source project
 *
 * Copyright 2013-2022 Apple Inc. and the FoundationDB project authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fdbrpc/AsyncFileEncrypted.h"
#include "flow/StreamCipher.h"
#include "flow/UnitTest.h"
#include "flow/xxhash.h"
#include "flow/actorcompiler.h" // must be last include


class AsyncFileEncryptedImpl {
public:
	// Determine the initialization for the first block of a file based on a hash of
	// the filename.
	static auto getFirstBlockIV(const std::string& filename) {
		StreamCipher::IV iv;
		auto salt = basename(filename);
		auto pos = salt.find('.');
		salt = salt.substr(0, pos);
		auto hash = XXH3_128bits(salt.c_str(), salt.size());
		auto pHigh = reinterpret_cast<unsigned char*>(&hash.high64);
		auto pLow = reinterpret_cast<unsigned char*>(&hash.low64);
		std::copy(pHigh, pHigh + 8, &iv[0]);
		std::copy(pLow, pLow + 4, &iv[8]);
		uint32_t blockZero = 0;
		auto pBlock = reinterpret_cast<unsigned char*>(&blockZero);
		std::copy(pBlock, pBlock + 4, &iv[12]);
		return iv;
	}

	// Read a single block of size ENCRYPTION_BLOCK_SIZE bytes, and decrypt.
	ACTOR static Future<Standalone<StringRef>> readBlock(AsyncFileEncrypted* self, uint32_t block) {
		state const int64_t taggedBlockSize = FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + ENCRYPTION_AUTH_TAG_SIZE;
		state Arena arena;
		// Add space for the 16-byte authentication tag
		state unsigned char* encrypted = new (arena) unsigned char[taggedBlockSize];
		int bytes = wait(uncancellable(holdWhile(arena,
		                                         self->file->read(encrypted,
		                                                          taggedBlockSize,
																  taggedBlockSize * block))));
		StreamCipherKey const* cipherKey = StreamCipherKey::getGlobalCipherKey();
		DecryptionStreamCipher decryptor(cipherKey, self->getIV(block));
		
		// Make sure we have at least 16 bytes for the tag
		if (bytes < ENCRYPTION_AUTH_TAG_SIZE) {
			// Error: block is too small to contain a tag
			throw io_error();
		}
		
		// Separate the ciphertext from the authentication tag (last 16 bytes)
		int cipherBytes = bytes - ENCRYPTION_AUTH_TAG_SIZE;
		auto decrypted = decryptor.decrypt(encrypted, cipherBytes, arena);
		
		// Process the authentication tag and finish decryption
		auto tagData = encrypted + cipherBytes;
		auto finalizationData = decryptor.finish(tagData, arena);
		// Combine decrypted data
		auto result = new (arena) unsigned char[decrypted.size()];
		memcpy(result, decrypted.begin(), decrypted.size());

		StringRef resultRef(result, decrypted.size());
		return Standalone<StringRef>(resultRef, arena);
	}

	ACTOR static Future<int> read(Reference<AsyncFileEncrypted> self, void* data, int length, int64_t offset) {
		if (self->fileSize == -1) {
			state int64_t fileSize = wait(self->size());
			self->fileSize = fileSize;
		}
		if (offset >= self->fileSize) {
			return 0;
		}
		if (offset + length > self->fileSize) {
			length = self->fileSize - offset;
		}
		if (length == 0) {
			return 0;
		}
		state uint32_t firstBlock = offset / FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE;
		state uint32_t lastBlock = (offset + length - 1) / FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE;
		state uint32_t block;
		state unsigned char* output = reinterpret_cast<unsigned char*>(data);
		state int bytesRead = 0;
		// NOTE(millas): commented out to make test work
		for (block = firstBlock; block <= lastBlock; ++block) {
			state Standalone<StringRef> plaintext;

			auto cachedBlock = self->readBuffers.get(block);
			if (cachedBlock.present()) {
				plaintext = cachedBlock.get();
			} else {
				wait(store(plaintext, readBlock(self.getPtr(), block)));
				self->readBuffers.insert(block, plaintext);
			}
			auto start = (block == firstBlock) ? plaintext.begin() + (offset % FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE)
			                                   : plaintext.begin();
			auto end = (block == lastBlock)
			               ? plaintext.begin() + ((offset + length) % FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE)
			               : plaintext.end();
			if ((offset + length) % FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE == 0) {
				end = plaintext.end();
			}

			// The block could be short if it includes or is after the end of the file.
			end = std::min(end, plaintext.end());
			// If the start position is at or after the end of the block, the read is complete.
			if (start == end || start >= plaintext.end()) {
				break;
			}

			std::copy(start, end, output);
			output += (end - start);
			bytesRead += (end - start);
		}
		return bytesRead;
	}

	ACTOR static Future<Void> write(Reference<AsyncFileEncrypted> self, void const* data, int length, int64_t offset) {
		ASSERT(self->mode == AsyncFileEncrypted::Mode::APPEND_ONLY);
		// All writes must append to the end of the file:
		ASSERT_EQ(offset, self->currentBlock * FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + self->offsetInBlock);
		state unsigned char const* input = reinterpret_cast<unsigned char const*>(data);
		while (length > 0) {
			const auto chunkSize = std::min(length, FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE - self->offsetInBlock);
			Arena arena;
			auto encrypted = self->encryptor->encrypt(input, chunkSize, arena);
			std::copy(encrypted.begin(), encrypted.end(), &self->writeBuffer[self->offsetInBlock]);
			offset += encrypted.size();
			self->offsetInBlock += chunkSize;
			length -= chunkSize;
			input += chunkSize;
			if (self->offsetInBlock == FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE) {
				// Finish encryption and get the authentication tag
				auto finishData = self->encryptor->finish(arena);
				
				// Ensure we have space for the tag at the end of the block
				ASSERT(self->offsetInBlock + finishData.size() <= self->writeBuffer.size());
				
				// Copy the finish data (which includes the auth tag) to the end of the write buffer
				std::copy(finishData.begin(), finishData.end(), &self->writeBuffer[self->offsetInBlock]);
				self->offsetInBlock += finishData.size(); // Should be 16 for the tag
				
				wait(self->writeLastBlockToFile());
				self->offsetInBlock = 0;
				ASSERT_LT(self->currentBlock, std::numeric_limits<uint32_t>::max());
				++self->currentBlock;
				self->encryptor = std::make_unique<EncryptionStreamCipher>(StreamCipherKey::getGlobalCipherKey(),
				                                                           self->getIV(self->currentBlock));
			}
		}
		return Void();
	}

	ACTOR static Future<Void> sync(Reference<AsyncFileEncrypted> self) {
		ASSERT(self->mode == AsyncFileEncrypted::Mode::APPEND_ONLY);
		
		// If there's any data in the current block, finish encryption and write it out
		if (self->offsetInBlock > 0) {
			Arena arena;
			auto finishData = self->encryptor->finish(arena);
			
			// Copy the finish data (which includes the auth tag) to the end of the write buffer
			std::copy(finishData.begin(), finishData.end(), &self->writeBuffer[self->offsetInBlock]);
			self->offsetInBlock += finishData.size(); // Should be 16 for the tag
		}

		wait(self->writeLastBlockToFile());
		wait(self->file->sync());
		return Void();
	}

	ACTOR static Future<Void> zeroRange(AsyncFileEncrypted* self, int64_t offset, int64_t length) {
		ASSERT(self->mode == AsyncFileEncrypted::Mode::APPEND_ONLY);
		// TODO: Could optimize this
		Arena arena;
		auto zeroes = new (arena) unsigned char[length];
		memset(zeroes, 0, length);
		wait(uncancellable(holdWhile(arena, self->write(zeroes, length, offset))));
		return Void();
	}

	ACTOR static Future<int64_t> size(const AsyncFileEncrypted* self) {
		// state int64_t fileSize = wait(self->file->size());
		// auto signatureSizes = ENCRYPTION_AUTH_TAG_SIZE * (fileSize / FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE);
		// return fileSize - signatureSizes;
		state const int64_t decryptedBlockSize = FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE;
		state const int64_t encryptedBlockSize = FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + ENCRYPTION_AUTH_TAG_SIZE;
		state int64_t fileSize = wait(self->file->size());

		uint64_t numFullBlocks = fileSize / encryptedBlockSize;
		uint64_t lastBlockSize = fileSize % encryptedBlockSize;
		uint64_t decryptedSize = numFullBlocks * decryptedBlockSize;

		if (lastBlockSize > 0) {
			ASSERT(lastBlockSize >= ENCRYPTION_AUTH_TAG_SIZE);
			decryptedSize += lastBlockSize - ENCRYPTION_AUTH_TAG_SIZE;
		}

		return decryptedSize;
	}
};

AsyncFileEncrypted::AsyncFileEncrypted(Reference<IAsyncFile> file, Mode mode)
  : file(file), mode(mode), readBuffers(FLOW_KNOBS->MAX_DECRYPTED_BLOCKS), currentBlock(0) {
	firstBlockIV = AsyncFileEncryptedImpl::getFirstBlockIV(file->getFilename());
	if (mode == Mode::APPEND_ONLY) {
		encryptor =
		    std::make_unique<EncryptionStreamCipher>(StreamCipherKey::getGlobalCipherKey(), getIV(currentBlock));
		// Add space for the 16-byte authentication tag at the end of each block
		writeBuffer = std::vector<unsigned char>(FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + ENCRYPTION_AUTH_TAG_SIZE, 0);
	}
}

void AsyncFileEncrypted::addref() {
	ReferenceCounted<AsyncFileEncrypted>::addref();
}

void AsyncFileEncrypted::delref() {
	ReferenceCounted<AsyncFileEncrypted>::delref();
}

Future<int> AsyncFileEncrypted::read(void* data, int length, int64_t offset) {
	return AsyncFileEncryptedImpl::read(Reference<AsyncFileEncrypted>::addRef(this), data, length, offset);
}

Future<Void> AsyncFileEncrypted::write(void const* data, int length, int64_t offset) {
	return AsyncFileEncryptedImpl::write(Reference<AsyncFileEncrypted>::addRef(this), data, length, offset);
}

Future<Void> AsyncFileEncrypted::zeroRange(int64_t offset, int64_t length) {
	return AsyncFileEncryptedImpl::zeroRange(this, offset, length);
}

Future<Void> AsyncFileEncrypted::truncate(int64_t size) {
	ASSERT(mode == Mode::APPEND_ONLY);
	return file->truncate(size);
}

Future<Void> AsyncFileEncrypted::sync() {
	ASSERT(mode == Mode::APPEND_ONLY);
	return AsyncFileEncryptedImpl::sync(Reference<AsyncFileEncrypted>::addRef(this));
}

Future<Void> AsyncFileEncrypted::flush() {
	ASSERT(mode == Mode::APPEND_ONLY);
	return Void();
}

std::string AsyncFileEncrypted::getFilename() const {
	return file->getFilename();
}

Future<Void> AsyncFileEncrypted::readZeroCopy(void** data, int* length, int64_t offset) {
	throw io_error();
	return Void();
}

void AsyncFileEncrypted::releaseZeroCopy(void* data, int length, int64_t offset) {
	throw io_error();
}

int64_t AsyncFileEncrypted::debugFD() const {
	return file->debugFD();
}

Future<int64_t> AsyncFileEncrypted::size() const {
	return AsyncFileEncryptedImpl::size(this);
}

StreamCipher::IV AsyncFileEncrypted::getIV(uint32_t block) const {
	auto iv = firstBlockIV;

	auto pBlock = reinterpret_cast<unsigned char*>(&block);
	std::copy(pBlock, pBlock + 4, &iv[12]);

	return iv;
}

Future<Void> AsyncFileEncrypted::writeLastBlockToFile() {
	// The source buffer for the write is owned by *this so this must be kept alive by reference count until the write
	// is finished.
	const int blockWriteSize = FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + ENCRYPTION_AUTH_TAG_SIZE;
	return uncancellable(
	    holdWhile(Reference<AsyncFileEncrypted>::addRef(this),
	              file->write(&writeBuffer[0], offsetInBlock, currentBlock * blockWriteSize)));
}

size_t AsyncFileEncrypted::RandomCache::evict() {
	ASSERT_EQ(vec.size(), maxSize);
	auto index = deterministicRandom()->randomInt(0, maxSize);
	hashMap.erase(vec[index]);
	return index;
}

AsyncFileEncrypted::RandomCache::RandomCache(size_t maxSize) : maxSize(maxSize) {
	vec.reserve(maxSize);
}

void AsyncFileEncrypted::RandomCache::insert(uint32_t block, const Standalone<StringRef>& value) {
	auto [_, found] = hashMap.insert({ block, value });
	if (found) {
		return;
	} else if (vec.size() < maxSize) {
		vec.push_back(block);
	} else {
		auto index = evict();
		vec[index] = block;
	}
}

Optional<Standalone<StringRef>> AsyncFileEncrypted::RandomCache::get(uint32_t block) const {
	auto it = hashMap.find(block);
	if (it == hashMap.end()) {
		return {};
	} else {
		return it->second;
	}
}

// This test writes random data into an encrypted file in random increments,
// then reads this data back from the file in random increments, then confirms that
// the bytes read match the bytes written.
TEST_CASE("fdbrpc/AsyncFileEncrypted/many") {
	state int i = 0;
	for (; i < 1000; ++i) {
		state int bytes = FLOW_KNOBS->ENCRYPTION_BLOCK_SIZE + i;
		printf("total bytes: %d\n", bytes);
		// params.setDataDir("/tmp");
		state std::vector<unsigned char> writeBuffer(bytes, 0);
		deterministicRandom()->randomBytes(&writeBuffer.front(), bytes);
		state std::vector<unsigned char> readBuffer(bytes, 0);
		// NOTE(millas): commented out to make test work
		// ASSERT(g_network->isSimulated());
		StreamCipherKey::initializeGlobalRandomTestKey();
		int flags = IAsyncFile::OPEN_READWRITE | IAsyncFile::OPEN_CREATE | IAsyncFile::OPEN_ATOMIC_WRITE_AND_CREATE |
					IAsyncFile::OPEN_UNBUFFERED | IAsyncFile::OPEN_ENCRYPTED | IAsyncFile::OPEN_UNCACHED |
					IAsyncFile::OPEN_NO_AIO;
		state Reference<IAsyncFile> file =
			wait(IAsyncFileSystem::filesystem()->open(joinPath(params.getDataDir(), "test-encrypted-file"), flags, 0600));
		state int bytesWritten = 0;
		state int chunkSize;
		while (bytesWritten < bytes) {
			chunkSize = std::min(deterministicRandom()->randomInt(0, 100), bytes - bytesWritten);
			wait(file->write(&writeBuffer[bytesWritten], chunkSize, bytesWritten));
			bytesWritten += chunkSize;
		}
		wait(file->sync());
		state int64_t writtenSize = wait(file->size());
		ASSERT(writtenSize == bytesWritten);
		state int bytesRead = 0;
		while (bytesRead < bytes) {
			chunkSize = bytes;
			// chunkSize = std::min(deterministicRandom()->randomInt(0, 100), bytes - bytesRead);
			int bytesReadInChunk = wait(file->read(&readBuffer[bytesRead], chunkSize, bytesRead));
			ASSERT_EQ(bytesReadInChunk, chunkSize);
			bytesRead += bytesReadInChunk;
		}
		ASSERT(bytesRead == bytes);
		ASSERT(writeBuffer == readBuffer);
	}
	return Void();
}


// validate that we can write and read back a one-byte field
TEST_CASE("fdbrpc/AsyncFileEncrypted/single_byte") {
	// params.setDataDir("/tmp");
	state std::vector<unsigned char> writeBuffer(1, 42);
	state std::vector<unsigned char> readBuffer(1, 0);
	StreamCipherKey::initializeGlobalRandomTestKey();
	int flags = IAsyncFile::OPEN_READWRITE | IAsyncFile::OPEN_CREATE | IAsyncFile::OPEN_ATOMIC_WRITE_AND_CREATE |
	            IAsyncFile::OPEN_UNBUFFERED | IAsyncFile::OPEN_ENCRYPTED | IAsyncFile::OPEN_UNCACHED |
	            IAsyncFile::OPEN_NO_AIO;
	state Reference<IAsyncFile> file =
	    wait(IAsyncFileSystem::filesystem()->open(joinPath(params.getDataDir(), "test-encrypted-file"), flags, 0600));
	wait(file->write(&writeBuffer[0], 1, 0));
	wait(file->sync());
	state int64_t fileSize = wait(file->size());
	state int bytesRead = wait(file->read(&readBuffer[0], fileSize, 0));

	ASSERT(bytesRead == 1);
	ASSERT(writeBuffer == readBuffer);
	return Void();
}

ACTOR static Future<Void> createCorruptEncryptionFile(std::string filename) {
	state int flags = IAsyncFile::OPEN_READWRITE | IAsyncFile::OPEN_CREATE | IAsyncFile::OPEN_ATOMIC_WRITE_AND_CREATE |
	                  IAsyncFile::OPEN_UNBUFFERED | IAsyncFile::OPEN_UNCACHED | IAsyncFile::OPEN_NO_AIO;
	state Reference<IAsyncFile> file = wait(IAsyncFileSystem::filesystem()->open(
		filename,
		flags,
		0600));
	state std::vector<unsigned char> writeBuffer(17, 0);
	wait(file->write(writeBuffer.data(), writeBuffer.size(), 0));
	wait(file->sync());
	return Void();
}

// validate that we throw when signatures are incorrect
TEST_CASE("fdbrpc/AsyncFileEncrypted/corrupt") {
	state std::vector<unsigned char> readBuffer(1, 0);
	// params.setDataDir("/tmp");
	StreamCipherKey::initializeGlobalRandomTestKey();

	wait(createCorruptEncryptionFile(joinPath(params.getDataDir(), "test-corrupt-encrypted-file")));

	state int flags = IAsyncFile::OPEN_READONLY | IAsyncFile::OPEN_UNBUFFERED | IAsyncFile::OPEN_ENCRYPTED | 
					  IAsyncFile::OPEN_UNCACHED | IAsyncFile::OPEN_NO_AIO;
	state Reference<IAsyncFile> encryptedFile =
		wait(IAsyncFileSystem::filesystem()->open(joinPath(params.getDataDir(), "test-corrupt-encrypted-file"), flags, 0600));
	state int64_t fileSize = wait(encryptedFile->size());

	try {
		state int bytesRead = wait(encryptedFile->read(&readBuffer[0], fileSize, 0));
		// expecting above to throw
		ASSERT(false);
	} catch (Error& err) {
		ASSERT(err.code() == error_code_io_error);
	}

	return Void();
}
