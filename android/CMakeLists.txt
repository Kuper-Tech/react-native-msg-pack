project(msg_pack)
cmake_minimum_required(VERSION 3.4.1)

set(PACKAGE_NAME "msg_pack")
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

include("${NODE_MODULES_DIR}/react-native/ReactAndroid/cmake-utils/folly-flags.cmake")
add_compile_options(${folly_FLAGS})

add_library(${PACKAGE_NAME}
            SHARED
            ../cpp/react-native-msg-pack.cpp
            ../cpp/mpack.c
            cpp-adapter.cpp)

target_include_directories(
  ${PACKAGE_NAME}
  PRIVATE
  "${NODE_MODULES_DIR}/react-native/React"
  "${NODE_MODULES_DIR}/react-native/React/Base"
  "${NODE_MODULES_DIR}/react-native/ReactCommon"
  "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
  "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
  "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/react/turbomodule"
  "${NODE_MODULES_DIR}/react-native/ReactCommon/runtimeexecutor"
  "../cpp"
  "./"
)

find_library(z-lib z)
find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)
find_package(hermes-engine REQUIRED CONFIG)

include_directories(
            ../cpp
)

target_link_libraries(
  ${PACKAGE_NAME}
  ${z-lib}
  ReactAndroid::folly_runtime
  ReactAndroid::glog
  ReactAndroid::jsi
  ReactAndroid::reactnativejni
  hermes-engine::libhermes
  android
  fbjni::fbjni
)
