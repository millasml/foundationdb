## Run command

JAVA_HOME=/System/Volumes/Data/Users/millas/Library/Java/JavaVirtualMachines/corretto-23.0.2/Contents/Home cmake -DUSE_WERROR=ON -DFORCE_BOOST_BUILD=ON -DCMAKE_CXX_FLAGS="-DBOOST_ASIO_ENABLE_OLD_SERVICES -DBOOST_BIND_GLOBAL_PLACEHOLDERS" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G Ninja ..

JAVA_HOME=/System/Volumes/Data/Users/millas/Library/Java/JavaVirtualMachines/corretto-23.0.2/Contents/Home cmake -DUSE_WERROR=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G Ninja ..

## Run Ninja
ninja

## Run into toml11 issue
Now we have a better understanding of the issue. The user tried to run `ninja` to build the project, but it failed because of a CMake version compatibility issue with the toml11 dependency. The error is:

```
CMake Error at CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.5 has been removed from CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.

  Or, add -DCMAKE_POLICY_VERSION_MINIMUM=3.5 to try configuring anyway.
```

update build/toml11Project-prefix/src/toml11Project/CMakeLists.txt
to use 3.5 first line


## Run Ninja again
ninja

## Run into boost issue
so boost succckkkss:
```
FAILED: [code=1] fdbrpc/CMakeFiles/fdbrpc_sampling.dir/Net2FileSystem.cpp.o 
/usr/bin/c++ -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED -DENABLE_SAMPLING -DFLOW_GRPC_ENABLED -DNO_INTELLISENSE -DPROTOBUF_USE_DLLS -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION -DCOMPILATION_UNIT=/Volumes/git/foundationdb/fdbrpc/Net2FileSystem.cpp -I/Volumes/git/foundationdb/fdbrpc/include -I/Volumes/git/foundationdb/build/fdbrpc/include -I/Volumes/git/foundationdb/fdbrpc/libeio -I/Volumes/git/foundationdb/build/generated -I/Volumes/git/foundationdb/flow/include -I/Volumes/git/foundationdb/build/flow/include -I/Volumes/git/foundationdb/contrib/libb64/include -I/Volumes/git/foundationdb/build/_deps/fmt-src/include -I/Volumes/git/foundationdb/contrib/SimpleOpt/include -I/Volumes/git/foundationdb/contrib/crc32/include -I/Volumes/git/foundationdb/contrib/md5/include -I/Volumes/git/foundationdb/contrib/rapidjson -isystem /opt/homebrew/include -isystem /opt/homebrew/Cellar/openssl@3/3.5.0/include -isystem /Volumes/git/foundationdb/build/boost_install/include -DBOOST_ASIO_ENABLE_OLD_SERVICES -DBOOST_BIND_GLOBAL_PLACEHOLDERS -stdlib=libc++ -O3 -DNDEBUG -std=c++20 -arch arm64 -DCMAKE_BUILD -fno-omit-frame-pointer -gdwarf-4 -ggdb1 -gz -Wall -Wextra -Wredundant-move -Wpessimizing-move -Woverloaded-virtual -Wshift-sign-overflow -Wno-sign-compare -Wno-undefined-var-template -Wno-unknown-warning-option -Wno-unused-parameter -Wno-constant-logical-operand -Wno-deprecated-copy -Wno-delete-non-abstract-non-virtual-dtor -Wno-range-loop-construct -Wno-reorder-ctor -Wno-unused-command-line-argument -Wno-ambiguous-reversed-operator -Wno-vla-cxx-extension -Wno-unused-function -Wno-unused-private-field -Wno-nullability-completeness -Wno-macro-redefined -Werror -Wno-error=format -Wunused-variable -Wno-deprecated -fvisibility=hidden -Wreturn-type -fPIC -DHAVE_OPENSSL -MD -MT fdbrpc/CMakeFiles/fdbrpc_sampling.dir/Net2FileSystem.cpp.o -MF fdbrpc/CMakeFiles/fdbrpc_sampling.dir/Net2FileSystem.cpp.o.d -o fdbrpc/CMakeFiles/fdbrpc_sampling.dir/Net2FileSystem.cpp.o -c /Volumes/git/foundationdb/fdbrpc/Net2FileSystem.cpp
In file included from /Volumes/git/foundationdb/fdbrpc/Net2FileSystem.cpp:44:
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:45:15: error: no type named 'io_service' in namespace 'boost::asio'
   45 |         boost::asio::io_service ios;
      |         ~~~~~~~~~~~~~^
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:46:15: error: no member named 'io_service' in namespace 'boost::asio'
   46 |         boost::asio::io_service::work
      |         ~~~~~~~~~~~~~^
/Volumes/git/foundationdb/fdbrpc/Net2FileSystem.cpp:83:32: error: no type named 'io_service' in namespace 'boost::asio'
   83 |                     static_cast<boost::asio::io_service*>((void*)g_network->global(INetwork::enASIOService)));
      |                                 ~~~~~~~~~~~~~^
3 errors generated.
[1140/1661] Building CXX object fdbrpc/CMakeFiles/fdbrpc_sampling.dir/SimExternalConnection.actor.g.cpp.o
FAILED: [code=1] fdbrpc/CMakeFiles/fdbrpc_sampling.dir/SimExternalConnection.actor.g.cpp.o 
/usr/bin/c++ -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED -DENABLE_SAMPLING -DFLOW_GRPC_ENABLED -DNO_INTELLISENSE -DPROTOBUF_USE_DLLS -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION -DCOMPILATION_UNIT=/Volumes/git/foundationdb/build/fdbrpc/SimExternalConnection.actor.g.cpp -I/Volumes/git/foundationdb/fdbrpc/include -I/Volumes/git/foundationdb/build/fdbrpc/include -I/Volumes/git/foundationdb/fdbrpc/libeio -I/Volumes/git/foundationdb/build/generated -I/Volumes/git/foundationdb/flow/include -I/Volumes/git/foundationdb/build/flow/include -I/Volumes/git/foundationdb/contrib/libb64/include -I/Volumes/git/foundationdb/build/_deps/fmt-src/include -I/Volumes/git/foundationdb/contrib/SimpleOpt/include -I/Volumes/git/foundationdb/contrib/crc32/include -I/Volumes/git/foundationdb/contrib/md5/include -I/Volumes/git/foundationdb/contrib/rapidjson -isystem /opt/homebrew/include -isystem /opt/homebrew/Cellar/openssl@3/3.5.0/include -isystem /Volumes/git/foundationdb/build/boost_install/include -DBOOST_ASIO_ENABLE_OLD_SERVICES -DBOOST_BIND_GLOBAL_PLACEHOLDERS -stdlib=libc++ -O3 -DNDEBUG -std=c++20 -arch arm64 -DCMAKE_BUILD -fno-omit-frame-pointer -gdwarf-4 -ggdb1 -gz -Wall -Wextra -Wredundant-move -Wpessimizing-move -Woverloaded-virtual -Wshift-sign-overflow -Wno-sign-compare -Wno-undefined-var-template -Wno-unknown-warning-option -Wno-unused-parameter -Wno-constant-logical-operand -Wno-deprecated-copy -Wno-delete-non-abstract-non-virtual-dtor -Wno-range-loop-construct -Wno-reorder-ctor -Wno-unused-command-line-argument -Wno-ambiguous-reversed-operator -Wno-vla-cxx-extension -Wno-unused-function -Wno-unused-private-field -Wno-nullability-completeness -Wno-macro-redefined -Werror -Wno-error=format -Wunused-variable -Wno-deprecated -fvisibility=hidden -Wreturn-type -fPIC -DHAVE_OPENSSL -MD -MT fdbrpc/CMakeFiles/fdbrpc_sampling.dir/SimExternalConnection.actor.g.cpp.o -MF fdbrpc/CMakeFiles/fdbrpc_sampling.dir/SimExternalConnection.actor.g.cpp.o.d -o fdbrpc/CMakeFiles/fdbrpc_sampling.dir/SimExternalConnection.actor.g.cpp.o -c /Volumes/git/foundationdb/build/fdbrpc/SimExternalConnection.actor.g.cpp
/Volumes/git/foundationdb/fdbrpc/SimExternalConnection.actor.cpp:44:8: error: unknown type name 'io_service'
   44 | static io_service ios;
      |        ^
/Volumes/git/foundationdb/fdbrpc/SimExternalConnection.actor.cpp:114:29: error: use of undeclared identifier 'mutable_buffers_1'; did you mean 'mutable_buffer'?
  114 |                 index += socket.read_some(mutable_buffers_1(&tempReadBuffer[index], bytesReadable), err);
      |                                           ^
/opt/homebrew/include/boost/asio/detail/is_buffer_sequence.hpp:26:7: note: 'mutable_buffer' declared here
   26 | class mutable_buffer;
      |       ^
/Volumes/git/foundationdb/fdbrpc/SimExternalConnection.actor.cpp:131:38: error: no member named 'to_ulong' in 'boost::asio::ip::address_v4'
  131 |                 return NetworkAddress(addr.to_v4().to_ulong(), endpoint.port());
      |                                       ~~~~~~~~~~~~ ^
/Volumes/git/foundationdb/fdbrpc/SimExternalConnection.actor.cpp:198:2: error: unknown type name 'io_service'
  198 |         io_service ios;
      |         ^
/Volumes/git/foundationdb/fdbrpc/SimExternalConnection.actor.cpp:205:34: error: use of undeclared identifier 'mutable_buffers_1'; did you mean 'mutable_buffer'?
  205 |                 auto length = socket.read_some(mutable_buffers_1(readBuffer, readBufferSize), err);
      |                                                ^
/opt/homebrew/include/boost/asio/detail/is_buffer_sequence.hpp:26:7: note: 'mutable_buffer' declared here
   26 | class mutable_buffer;
      |       ^
5 errors generated.
[1145/1661] Building CXX object fdbrpc/CMakeFiles/fdbrpc_sampling.dir/swift_sim2_hooks.cpp.o
FAILED: [code=1] fdbrpc/CMakeFiles/fdbrpc_sampling.dir/swift_sim2_hooks.cpp.o 
/usr/bin/c++ -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED -DENABLE_SAMPLING -DFLOW_GRPC_ENABLED -DNO_INTELLISENSE -DPROTOBUF_USE_DLLS -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION -DCOMPILATION_UNIT=/Volumes/git/foundationdb/fdbrpc/swift_sim2_hooks.cpp -I/Volumes/git/foundationdb/fdbrpc/include -I/Volumes/git/foundationdb/build/fdbrpc/include -I/Volumes/git/foundationdb/fdbrpc/libeio -I/Volumes/git/foundationdb/build/generated -I/Volumes/git/foundationdb/flow/include -I/Volumes/git/foundationdb/build/flow/include -I/Volumes/git/foundationdb/contrib/libb64/include -I/Volumes/git/foundationdb/build/_deps/fmt-src/include -I/Volumes/git/foundationdb/contrib/SimpleOpt/include -I/Volumes/git/foundationdb/contrib/crc32/include -I/Volumes/git/foundationdb/contrib/md5/include -I/Volumes/git/foundationdb/contrib/rapidjson -isystem /opt/homebrew/include -isystem /opt/homebrew/Cellar/openssl@3/3.5.0/include -isystem /Volumes/git/foundationdb/build/boost_install/include -DBOOST_ASIO_ENABLE_OLD_SERVICES -DBOOST_BIND_GLOBAL_PLACEHOLDERS -stdlib=libc++ -O3 -DNDEBUG -std=c++20 -arch arm64 -DCMAKE_BUILD -fno-omit-frame-pointer -gdwarf-4 -ggdb1 -gz -Wall -Wextra -Wredundant-move -Wpessimizing-move -Woverloaded-virtual -Wshift-sign-overflow -Wno-sign-compare -Wno-undefined-var-template -Wno-unknown-warning-option -Wno-unused-parameter -Wno-constant-logical-operand -Wno-deprecated-copy -Wno-delete-non-abstract-non-virtual-dtor -Wno-range-loop-construct -Wno-reorder-ctor -Wno-unused-command-line-argument -Wno-ambiguous-reversed-operator -Wno-vla-cxx-extension -Wno-unused-function -Wno-unused-private-field -Wno-nullability-completeness -Wno-macro-redefined -Werror -Wno-error=format -Wunused-variable -Wno-deprecated -fvisibility=hidden -Wreturn-type -fPIC -DHAVE_OPENSSL -MD -MT fdbrpc/CMakeFiles/fdbrpc_sampling.dir/swift_sim2_hooks.cpp.o -MF fdbrpc/CMakeFiles/fdbrpc_sampling.dir/swift_sim2_hooks.cpp.o.d -o fdbrpc/CMakeFiles/fdbrpc_sampling.dir/swift_sim2_hooks.cpp.o -c /Volumes/git/foundationdb/fdbrpc/swift_sim2_hooks.cpp
In file included from /Volumes/git/foundationdb/fdbrpc/swift_sim2_hooks.cpp:25:
In file included from /Volumes/git/foundationdb/flow/include/flow/swift_concurrency_hooks.h:27:
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:45:15: error: no type named 'io_service' in namespace 'boost::asio'
   45 |         boost::asio::io_service ios;
      |         ~~~~~~~~~~~~~^
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:46:15: error: no member named 'io_service' in namespace 'boost::asio'
   46 |         boost::asio::io_service::work
      |         ~~~~~~~~~~~~~^
2 errors generated.
[1150/1661] Building CXX object fdbrpc/CMakeFiles/fdbrpc_sampling.dir/sim2.actor.g.cpp.o
FAILED: [code=1] fdbrpc/CMakeFiles/fdbrpc_sampling.dir/sim2.actor.g.cpp.o 
/usr/bin/c++ -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED -DENABLE_SAMPLING -DFLOW_GRPC_ENABLED -DNO_INTELLISENSE -DPROTOBUF_USE_DLLS -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION -DCOMPILATION_UNIT=/Volumes/git/foundationdb/build/fdbrpc/sim2.actor.g.cpp -I/Volumes/git/foundationdb/fdbrpc/include -I/Volumes/git/foundationdb/build/fdbrpc/include -I/Volumes/git/foundationdb/fdbrpc/libeio -I/Volumes/git/foundationdb/build/generated -I/Volumes/git/foundationdb/flow/include -I/Volumes/git/foundationdb/build/flow/include -I/Volumes/git/foundationdb/contrib/libb64/include -I/Volumes/git/foundationdb/build/_deps/fmt-src/include -I/Volumes/git/foundationdb/contrib/SimpleOpt/include -I/Volumes/git/foundationdb/contrib/crc32/include -I/Volumes/git/foundationdb/contrib/md5/include -I/Volumes/git/foundationdb/contrib/rapidjson -isystem /opt/homebrew/include -isystem /opt/homebrew/Cellar/openssl@3/3.5.0/include -isystem /Volumes/git/foundationdb/build/boost_install/include -DBOOST_ASIO_ENABLE_OLD_SERVICES -DBOOST_BIND_GLOBAL_PLACEHOLDERS -stdlib=libc++ -O3 -DNDEBUG -std=c++20 -arch arm64 -DCMAKE_BUILD -fno-omit-frame-pointer -gdwarf-4 -ggdb1 -gz -Wall -Wextra -Wredundant-move -Wpessimizing-move -Woverloaded-virtual -Wshift-sign-overflow -Wno-sign-compare -Wno-undefined-var-template -Wno-unknown-warning-option -Wno-unused-parameter -Wno-constant-logical-operand -Wno-deprecated-copy -Wno-delete-non-abstract-non-virtual-dtor -Wno-range-loop-construct -Wno-reorder-ctor -Wno-unused-command-line-argument -Wno-ambiguous-reversed-operator -Wno-vla-cxx-extension -Wno-unused-function -Wno-unused-private-field -Wno-nullability-completeness -Wno-macro-redefined -Werror -Wno-error=format -Wunused-variable -Wno-deprecated -fvisibility=hidden -Wreturn-type -fPIC -DHAVE_OPENSSL -MD -MT fdbrpc/CMakeFiles/fdbrpc_sampling.dir/sim2.actor.g.cpp.o -MF fdbrpc/CMakeFiles/fdbrpc_sampling.dir/sim2.actor.g.cpp.o.d -o fdbrpc/CMakeFiles/fdbrpc_sampling.dir/sim2.actor.g.cpp.o -c /Volumes/git/foundationdb/build/fdbrpc/sim2.actor.g.cpp
In file included from /Volumes/git/foundationdb/fdbrpc/sim2.actor.cpp:56:
In file included from /Volumes/git/foundationdb/flow/include/flow/swift_concurrency_hooks.h:27:
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:45:15: error: no type named 'io_service' in namespace 'boost::asio'
   45 |         boost::asio::io_service ios;
      |         ~~~~~~~~~~~~~^
/Volumes/git/foundationdb/flow/include/flow/AsioReactor.h:46:15: error: no member named 'io_service' in namespace 'boost::asio'
   46 |         boost::asio::io_service::work
      |         ~~~~~~~~~~~~~^
2 errors generated.
```


```
Now we're seeing a different set of issues. The build is failing due to compatibility problems between the code and the version of Boost being used. The key errors are related to Boost ASIO:

1. `no type named 'io_service' in namespace 'boost::asio'`
2. `no member named 'io_service' in namespace 'boost::asio'`
3. `no member named 'to_ulong' in 'boost::asio::ip::address_v4'`
4. `use of undeclared identifier 'mutable_buffers_1'; did you mean 'mutable_buffer'`

This suggests that the code is using older Boost ASIO APIs that have been deprecated or renamed in the newer Boost versions. For example, in newer versions of Boost ASIO:

- `io_service` has been renamed to `io_context`
- `mutable_buffers_1` has been replaced with `mutable_buffer`
- `to_ulong()` method has been renamed or changed
```

- In `flow/include/flow/AsioReactor.h`:

  - Changed `boost::asio::io_service` to `boost::asio::io_context`
  - Changed `boost::asio::io_service::work` to `boost::asio::io_context::work`
  - Changed `boost::asio::mutable_buffers_1` to `boost::asio::mutable_buffer`

- In `fdbrpc/Net2FileSystem.cpp`:

  - Changed `boost::asio::io_service*` to `boost::asio::io_context*`

- In `fdbrpc/SimExternalConnection.actor.cpp`:

  - Changed `static io_service ios;` to `static io_context ios;`
  - Changed `io_service ios;` to `io_context ios;`
  - Changed all instances of `mutable_buffers_1` to `mutable_buffer`
  - Updated `addr.to_v4().to_ulong()` to `addr.to_v4().to_uint()`




## Running 
need bash 4.0 or newer


USE_S3=true bash fdbbackup/tests/s3_backup_test.sh /Volumes/git/foundationdb ./build


## side quest (do not bother): building boost:

I've successfully fixed the Boost build failure by addressing the C99 compliance issues. Here's what was done:

1. Identified the issue: The build was failing because of undeclared function calls in several source files, which newer C compilers (especially in macOS) treat as errors due to C99 standard compliance requirements.

2. Fixed the issue by adding the missing header file includes to the source files:

   - Added `#include "output.h"` to `tools/build/src/engine/execcmd.c`
   - Added `#include "output.h"` to `tools/build/src/engine/make.c`
   - Added `#include "../filesys.h"` to `tools/build/src/engine/modules/path.c`

3. The bootstrap script now completes successfully, creating both `b2` and `bjam` executables.

There are still some non-critical warnings about string operations in the code, but these are just warnings and don't prevent the build from completing.

You can now continue with building Boost itself by running:

```javascript
./b2 --prefix=/opt/homebrew/Cellar/boost/1.60.0 install
```

This will compile and install all Boost libraries to your specified prefix directory.
