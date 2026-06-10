#!/usr/bin/env sh
set -e
set -o pipefail

PIO_ENVIRONMENT="native-fuzz"
BUILD_DIR=".pio/build/${PIO_ENVIRONMENT}"

echo "Building fuzzer..."
pio run -e ${PIO_ENVIRONMENT}

echo "Running fuzzer for 60 seconds..."
ASAN_OPTIONS=detect_leaks=1 \
  LSAN_OPTIONS=suppressions=test/fuzz/lsan_suppressions.txt \
  LLVM_PROFILE_FILE=${BUILD_DIR}/program.profraw \
  ${BUILD_DIR}/program -max_len=128 -prefer_small=1 -rss_limit_mb=512 -max_total_time=300 -use_value_profile=1 test/fuzz/corpus/generated test/fuzz/corpus/found test/fuzz/corpus/known

echo "Generating coverage report..."
llvm-profdata merge -sparse ${BUILD_DIR}/program.profraw -o ${BUILD_DIR}/program.profdata
llvm-cov show ${BUILD_DIR}/program -instr-profile=${BUILD_DIR}/program.profdata -format=html -output-dir=${BUILD_DIR}/coverage

echo "Fuzzing complete. Coverage report generated at ${BUILD_DIR}/coverage/index.html"
