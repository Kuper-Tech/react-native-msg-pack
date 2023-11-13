#ifndef MSGPACK_H
#define MSGPACK_H
#include "ReactCommon/CallInvoker.h"
#include <jsi/jsi.h>
#include <unordered_map>
#include "defines.h"
#include "parser.h"

using namespace facebook;

class SharedBuffer : public jsi::MutableBuffer {
public:
  explicit SharedBuffer(const u8 *data, u32 size) : data_(data), size_(size) {}

  uint8_t *data() override { return const_cast<uint8_t *>(data_); };
  size_t size() const override { return size_; };

  const u8 *data_;
  u32 size_;
};

typedef struct values_vec_t values_vec_t;
struct values_vec_t {
    jsi::Value* values;
    u32 size;
};

typedef struct writter_ctx_t writter_ctx_t;
struct writter_ctx_t {
  std::unordered_map<std::string, u32> stringsCache;
  std::vector<jsi::Value> values;
  jsi::Runtime *rt;
};

static inline u64 make_plain_value(void *ctx, jsi::Value val) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  u64 id = writter->values.size();
  writter->values.push_back(std::move(val));
  return id;
}

static inline u64 make_null(void *ctx) {
  return make_plain_value(ctx, jsi::Value(nullptr));
}

static u64 make_bool(void *ctx, bool value) {
  return make_plain_value(ctx, jsi::Value(value));
}

static u64 make_int(void *ctx, i64 value) {
  return make_plain_value(ctx, jsi::Value((double)value));
}

static u64 make_uint(void *ctx, u64 value) {
  return make_plain_value(ctx, jsi::Value((double)value));
}

static u64 make_float(void *ctx, f32 value) {
  return make_plain_value(ctx, jsi::Value((double) value));
}

static u64 make_double(void *ctx, f64 value) {
  return make_plain_value(ctx, jsi::Value((double)value));
}

static u64 make_string(void *ctx, const u8 *data, u32 length) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  std::string str = std::string((char *)data, length);
  if (writter->stringsCache.count(str) > 0) {
    return writter->stringsCache[str];
  } else {
    u64 id = writter->values.size();
    jsi::Runtime &rt = *writter->rt;
    writter->values.emplace_back(jsi::String::createFromUtf8(rt, data, length));
    writter->stringsCache[str] = id;
    return id;
  }
}

static u64 make_bin(void *ctx, const u8 *data, u32 length) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  u64 id = writter->values.size();
  jsi::Runtime &rt = *writter->rt;
  auto val = std::make_shared<SharedBuffer>(data, length);
  writter->values.emplace_back(jsi::ArrayBuffer(rt, val));
  return id;
}

static u64 make_map(void *ctx) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  u64 id = writter->values.size();
  jsi::Runtime &rt = *writter->rt;
  writter->values.emplace_back(jsi::Object(rt));
  return id;
}

static u64 make_array(void *ctx, u32 length) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  u64 id = writter->values.size();
  jsi::Runtime &rt = *writter->rt;
  writter->values.emplace_back(jsi::Array(rt, length));
  return id;
}

static void map_put(void *ctx, u64 parent, u64 key, u64 value) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  jsi::Runtime &rt = *writter->rt;
  writter->values[parent]
    .asObject(rt)
    .setProperty(
      rt, writter->values[key].toString(rt),
      writter->values[value]
    );
}

static void array_add(void *ctx, u64 parent, u64 item, u32 index) {
  writter_ctx_t *writter = (writter_ctx_t *)ctx;
  jsi::Runtime &rt = *writter->rt;
  assert(writter->values[parent].asObject(rt).isArray(rt));
  writter->values[parent]
    .asObject(rt)
    .asArray(rt)
    .setValueAtIndex(rt, index, writter->values[item]);
}

static sax_callbacks_t callbacks{
    make_null,   make_bool, make_int, make_uint,  make_float, make_double,
    make_string, make_bin,  make_map, make_array, map_put,    array_add};

static jsi::Value parse(jsi::Runtime *rt, const u8 *data, u32 size) {
  writter_ctx_t ctx;
  ctx.rt = rt;
  bool result = parse_msg_pack(data, size, &callbacks, (void *)&ctx);
  
  return result ? std::move(ctx.values[0]) : jsi::Value(nullptr);
}

static void encode_element(mpack_writer_t *writter, jsi::Runtime &rt,
                           jsi::Value *val) {
  if (val->isNumber()) {
    mpack_write_double(writter, val->getNumber());
    return;
  }
  if (val->isBool()) {
    mpack_write_bool(writter, val->getBool());
    return;
  }
  if (val->isNull()) {
    mpack_write_nil(writter);
    return;
  }
  if (val->isString()) {
    mpack_write_cstr(writter, val->toString(rt).utf8(rt).c_str());
    return;
  }
  if (val->isObject()) {
    if (val->asObject(rt).isArray(rt)) {
      mpack_build_array(writter);
      u32 count = val->asObject(rt).asArray(rt).length(rt);
      for (u32 i = 0; i < count; i++) {
        jsi::Value v = val->asObject(rt).asArray(rt).getValueAtIndex(rt, i);
        encode_element(writter, rt, &v);
      }
      mpack_complete_array(writter);
    } else {
      mpack_build_map(writter);
      jsi::Array propertyNames = val->asObject(rt).getPropertyNames(rt);
      u32 count = propertyNames.length(rt);
      for (u32 i = 0; i < count; i++) {
        jsi::Value key = propertyNames.getValueAtIndex(rt, i);
        jsi::String keyStr = key.toString(rt);
        encode_element(writter, rt, &key);
        jsi::Value v = val->asObject(rt).getProperty(rt, keyStr);
        encode_element(writter, rt, &v);
      }
      mpack_complete_map(writter);
    }
  }
}

static jsi::Value encode(jsi::Runtime &runtime, jsi::Value *val) {
  mpack_writer_t writter;
  u8 *data;
  size_t size;
  mpack_writer_init_growable(&writter, (char **)&data, &size);
  encode_element(&writter, runtime, val);
  i32 res = mpack_writer_destroy(&writter);
  if (res != mpack_ok) {
    fprintf(stderr, "An error occurred encoding the data!\n");
    return jsi::Value((double)res);
  }
  return jsi::Value(
      jsi::ArrayBuffer(runtime, std::make_shared<SharedBuffer>(data, size)));
}

namespace msg_pack {
int install(jsi::Runtime *runtime, facebook::react::CallInvoker *callinvoker);

} // namespace rn_curl

#endif
