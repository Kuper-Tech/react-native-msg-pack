import { NativeModules } from 'react-native';

let __installed = false;
export function install() {
  if (__installed) {
    return;
  }
  NativeModules.MsgPack.install();
  __installed = true;
}
declare global {
  var __parseMsgPack: (ar: ArrayBuffer) => unknown;
  var __encodeMsgPack: (t: unknown) => ArrayBuffer;
}

export function parse<T = unknown>(ab: ArrayBuffer): T {
  return __parseMsgPack(ab) as T;
}

export function encode<T = unknown>(t: T): ArrayBuffer {
  return __encodeMsgPack(t);
}
