
#include "react-native-msg-pack.h"
#include <ReactCommon/CallInvokerHolder.h>
#include <fbjni/detail/Registration.h>
#include <jni.h>
#include <jsi/jsi.h>

using namespace facebook;

struct RNCurlModule : jni::JavaClass<RNCurlModule> {

public:
  __unused static constexpr auto kJavaDescriptor =
      "Lcom/msgpack/MsgPackModule;";

  static void registerNatives() {
    javaClassStatic()->registerNatives({
        makeNativeMethod("installNative", RNCurlModule::installNative)
    });
  }

private:
  static int installNative(
      jni::alias_ref<jni::JClass>,
      jlong jsiRuntimePointer,
      jni::alias_ref<facebook::react::CallInvokerHolder::javaobject> jsCallInvokerHolder
  ) {
    auto runtime = reinterpret_cast<jsi::Runtime*>(jsiRuntimePointer);
    auto callInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();
    return msg_pack::install(runtime, callInvoker.get());
  }

};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  return facebook::jni::initialize(vm, [] {
      RNCurlModule::registerNatives();
  });
}

