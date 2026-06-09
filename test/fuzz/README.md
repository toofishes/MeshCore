# MeshCore Fuzz Testing

This directory hosts a fuzz testing harness that uses [libFuzzer](https://llvm.org/docs/LibFuzzer.html) to enable fuzz testing of parsing routines within MeshCore that parse input data - e.g. received radio packets.

The function entrypoint is `LLVMFuzzerTestOneInput`, which is a well-known function name to the fuzzing library.

## Running

You must have a version of LLVM installed and available on your path that provides the fuzzing library. The built-in version on OS X will not work; you will need to have LLVM installed from homebrew or similar.

Ensure the correct version of LLVM is available and takes precedence in PATH. E.g.:

```sh
PATH=/opt/homebrew/opt/llvm/bin:$PATH
```

Then run:

```sh
./test/fuzz/run.sh
```

This will kick off a `pio` build of the `native-fuzz` environment, which is defined in `platformio.ini` in this directory. It will then run the fuzzer for 60 seconds, and produce a code coverage report at the end.

If the fuzzer finds an input that trips up one of the sanitizers, causes a program crash, or something else, it will exit immediately and write the failing input out to disk with a filename like `crash-<sha1>`.

### Seeding with a corpus

If you happen to have a bunch of real messages, put them as raw bytes, one per file, in the `corpus/known/` directory. This will allow the fuzzer to have better seed data and starting points for valid packets so it is more likely to explore deeper coverage paths.

### Past found issues

Crashes and other inputs that caused issues in the past and have been fixed are located in the `corpus/found` directory. These are used as starting values for new searches in addition to any known packets as noted above.
