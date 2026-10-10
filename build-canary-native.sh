#!/usr/bin/env bash
set -euo pipefail
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
task_sdk="${TS_SDK_ROOT:-$HOME/android-sdk}"
task_cxx="$task_root/toolchain/libcxx/prefab/modules/cxx"
test -f "$task_cxx/libs/android.arm64-v8a/libcxx.a"
mkdir -p "$task_root/toolchain/cmake"
cat > "$task_root/toolchain/cmake/cxxConfig.cmake" <<EOF
add_library(cxx::cxx STATIC IMPORTED)
set_target_properties(cxx::cxx PROPERTIES
  IMPORTED_LOCATION "$task_cxx/libs/android.arm64-v8a/libcxx.a"
  INTERFACE_INCLUDE_DIRECTORIES "$task_cxx/include")
EOF
cmake -S "$task_root/app/src/main/cpp" -B "$task_root/build/native-arm64" \
  -DCMAKE_TOOLCHAIN_FILE="$task_sdk/ndk/29.0.14206865/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DANDROID_STL=none \
  -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON -DANDROID_ALLOW_UNDEFINED_SYMBOLS=ON \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=23 -DCMAKE_C_STANDARD=23 \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
  '-DCMAKE_CXX_FLAGS=-fno-exceptions -fno-rtti -fvisibility=hidden -fvisibility-inlines-hidden' \
  -Dcxx_DIR="$task_root/toolchain/cmake"
cmake --build "$task_root/build/native-arm64" --target TrickyStoreOSS --parallel 4
