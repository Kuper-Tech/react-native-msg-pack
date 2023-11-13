package com.msgpack;

import android.util.Log;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.turbomodule.core.CallInvokerHolderImpl;
import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder;

@ReactModule(name = MsgPackModule.NAME)
public class MsgPackModule extends ReactContextBaseJavaModule {
  public static final String NAME = "MsgPack";

  public MsgPackModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  static {
    System.loadLibrary("msg_pack");
  }

  private static native int installNative(
    long jsiRuntimePointer,
    CallInvokerHolderImpl jsCallInvoker
  );

  @ReactMethod(isBlockingSynchronousMethod = true)
  public void install() {
    ReactApplicationContext context = getReactApplicationContext();

    CallInvokerHolderImpl holder = (CallInvokerHolderImpl) context
      .getCatalystInstance()
      .getJSCallInvokerHolder();

    int result = installNative(
      context.getJavaScriptContextHolder().get(),
      holder
    );
    if (result != 0) {
      Log.e("CURL_MODULE","Can't install native lib");
    }
  }
}
