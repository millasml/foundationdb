JAVA_HOME=/System/Volumes/Data/Users/millas/Library/Java/JavaVirtualMachines/corretto-23.0.2/Contents/Home cmake -DUSE_WERROR=ON -G Ninja .. 

JAVA_HOME=/System/Volumes/Data/Users/millas/Library/Java/JavaVirtualMachines/corretto-23.0.2/Contents/Home cmake -DUSE_WERROR=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G Ninja ..

build/toml11Project-prefix/src/toml11Project/CMakeLists.txt

Now we have a better understanding of the issue. The user tried to run `ninja` to build the project, but it failed because of a CMake version compatibility issue with the toml11 dependency. The error is:

```
CMake Error at CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.5 has been removed from CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.

  Or, add -DCMAKE_POLICY_VERSION_MINIMUM=3.5 to try configuring anyway.
```

brew install boost@1.60
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

seems to run fine with these errors though

need bash 4.0 or newer


USE_S3=true bash fdbbackup/tests/s3_backup_test.sh /Volumes/git/foundationdb ./build
