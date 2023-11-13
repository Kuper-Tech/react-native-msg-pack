#ifndef PARSER_H
#define PARSER_H
#include "mpack.h"
#include "defines.h"

typedef struct sax_callbacks_t {
    u64 (*make_null)(void* ctx);
    u64 (*make_bool)(void* ctx, bool value);
    u64 (*make_int)(void* ctx, i64 value);
    u64 (*make_uint)(void* ctx, u64 value);
    u64 (*make_float)(void* ctx, f32 value);
    u64 (*make_double)(void* ctx, f64 value);
    u64 (*make_string)(void* ctx, const u8 *data, u32 length);
    u64 (*make_bin)(void* ctx, const u8 *data, u32 length);
    u64 (*make_map)(void* ctx);
    u64 (*make_array)(void* ctx, u32 length);
    void (*map_put)(void* ctx, u64 parent, u64 key, u64 value);
    void (*array_add)(void* ctx, u64 parent, u64 item, u32 index);
    u64 (*make_key)(void* ctx, const u8 *data, u32 length);
} sax_callbacks_t;


typedef struct parse_result_t parse_result_t;
struct parse_result_t {
    u64 id;
    bool valid;
};

static parse_result_t parse_element(mpack_reader_t *reader, int depth,
                          const sax_callbacks_t *callbacks, void *context);

static bool parse_msg_pack(const u8 *data, size_t length,
                           const sax_callbacks_t *callbacks, void *context) {
  mpack_reader_t reader;
  mpack_reader_init_data(&reader, (char *)data, length);
  parse_element(&reader, 0, callbacks, context);
  return mpack_ok == mpack_reader_destroy(&reader);
}

static parse_result_t parse_element(mpack_reader_t *reader, int depth,
                          const sax_callbacks_t *callbacks, void *context) {
  if (depth >= 32) {
    mpack_reader_flag_error(reader, mpack_error_too_big);
    return (parse_result_t){.valid = false};
  }

  mpack_tag_t tag = mpack_read_tag(reader);
  if (mpack_reader_error(reader) != mpack_ok) {
      return (parse_result_t){.valid = false};
  }
  parse_result_t result = (parse_result_t){.valid = true};
  switch (mpack_tag_type(&tag)) {
  case mpack_type_nil:
    result.id = callbacks->make_null(context);
    return result;

  case mpack_type_bool:
    result.id = callbacks->make_bool(context, mpack_tag_bool_value(&tag) != 0);
    return result;
  case mpack_type_int:
    result.id = callbacks->make_int(context, mpack_tag_int_value(&tag));
    return result;
  case mpack_type_uint:
    result.id = callbacks->make_uint(context, mpack_tag_uint_value(&tag));
    return result;
  case mpack_type_float:
    result.id = callbacks->make_float(context, mpack_tag_float_value(&tag));
    return result;
  case mpack_type_double:
    result.id = callbacks->make_double(context, mpack_tag_double_value(&tag));
    return result;
  case mpack_type_str: {
    u32 length = mpack_tag_str_length(&tag);
    const u8 *data = (u8 *)mpack_read_bytes_inplace(reader, length);
    u64 id = callbacks->make_string(context, data, length);
    mpack_done_str(reader);
    result.id = id;
    return result;
  }
  case mpack_type_bin: {
    u32 length = mpack_tag_bin_length(&tag);
    const u8 *data = (u8 *)mpack_read_bytes_inplace(reader, length);
    result.id = callbacks->make_bin(context, data, length);
    mpack_done_bin(reader);
    return result;
  }
  case mpack_type_array: {
    u32 count = mpack_tag_array_count(&tag);
    u64 arrId = callbacks->make_array(context, count);
    parse_result_t element_result;
    u32 index = 0;
    while (index < count) {
      element_result = parse_element(reader, depth + 1, callbacks, context);
      if (element_result.valid) {
          callbacks->array_add(context, arrId, element_result.id, index);
      }
      if (mpack_reader_error(reader) != mpack_ok)
        break;
      index++;
    }
    mpack_done_array(reader);
    result.id = arrId;
    return result;
  }
  case mpack_type_map: {
    u32 count = mpack_tag_map_count(&tag);
    u64 mapId = callbacks->make_map(context);

    parse_result_t key_result;
    parse_result_t value_result;
    while (count-- > 0) {
        key_result = parse_element(reader, depth + 1, callbacks, context);
        value_result = parse_element(reader, depth + 1, callbacks, context);
        if (key_result.valid && value_result.valid) {
            callbacks->map_put(context, mapId, key_result.id, value_result.id);
        }
      if (mpack_reader_error(reader) != mpack_ok) // critical check!
        break;
    }
    mpack_done_map(reader);
    result.id = mapId;
    return result;
  }
  default:
    fprintf(stderr,
            "Error: type %s not implemented by this example SAX parser.\n",
            mpack_type_to_string(mpack_tag_type(&tag)));
    result.valid = false;
    return result;
  }
}

#endif
