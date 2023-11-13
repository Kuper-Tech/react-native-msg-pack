#include "react-native-msg-pack.h"
#include "ReactCommon/CallInvoker.h"
#include "jsi/jsi.h"

using namespace facebook;
namespace msg_pack {
int install(jsi::Runtime *runtime, facebook::react::CallInvoker *callinvoker) {
  auto parse_cb = [runtime](jsi::Runtime &rt, const jsi::Value &thisValue,
                            const jsi::Value *args,
                            size_t count) -> jsi::Value {
    jsi::ArrayBuffer ab = args[0].asObject(rt).getArrayBuffer(rt);
    return parse(runtime, ab.data(rt), ab.size(rt));
  };
  runtime->global().setProperty(
      *runtime, "__parseMsgPack",
      jsi::Function::createFromHostFunction(
          *runtime, jsi::PropNameID::forAscii(*runtime, "__parseMsgPack"), 1,
          std::move(parse_cb)));
  auto encode_cb = [](jsi::Runtime &rt, const jsi::Value &thisValue,
                      const jsi::Value *args, size_t count) -> jsi::Value {
    auto v = &args[0];
    return encode(rt, const_cast<jsi::Value *>(v));
  };
  runtime->global().setProperty(
      *runtime, "__encodeMsgPack",
      jsi::Function::createFromHostFunction(
          *runtime, jsi::PropNameID::forAscii(*runtime, "__encodeMsgPack"), 1,
          std::move(encode_cb)));
  return 0;
}
} // namespace msg_pack
