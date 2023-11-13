#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>
#import <React/RCTUtils.h>
#import <memory>
#import <ReactCommon/RCTTurboModule.h>
#import "MsgPack.h"

using namespace facebook;

@implementation MsgPack

RCT_EXPORT_MODULE()

+ (BOOL)requiresMainQueueSetup {
    return YES;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install)
{
    auto _bridge = [RCTBridge currentBridge];
    auto _cxxBridge = (RCTCxxBridge*)_bridge;
    if (_cxxBridge == nil) return @false;
    jsi::Runtime* _runtime = (jsi::Runtime*) _cxxBridge.runtime;
    if (_runtime == nil) return @false;
    rn_curl::install(_runtime, _bridge.jsCallInvoker.get());
    return @true;
}

@end
