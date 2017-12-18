#include <jsni.h>

int JSNIGetVersion(JSNIEnv*) { return JSNI_VERSION_2_1; }
bool JSNIRegisterMethod(JSNIEnv*, const JSValueRef, const char*, JSNICallback) { return false; }
int JSNIGetArgsLengthOfCallback(JSNIEnv*, JSNICallbackInfo) { return 0; }
JSValueRef JSNIGetArgOfCallback(JSNIEnv*, JSNICallbackInfo, int) { return nullptr; }
JSValueRef JSNIGetThisOfCallback(JSNIEnv*, JSNICallbackInfo) { return nullptr; }
void* JSNIGetDataOfCallback(JSNIEnv*, JSNICallbackInfo) { return nullptr; }
void JSNISetReturnValue(JSNIEnv*, JSNICallbackInfo, JSValueRef) {}
bool JSNIIsUndefined(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewUndefined(JSNIEnv*) { return nullptr; }
bool JSNIIsNull(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewNull(JSNIEnv*) { return nullptr; }
bool JSNIIsBoolean(JSNIEnv*, JSValueRef) { return false; }
bool JSNIToCBool(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewBoolean(JSNIEnv*, bool) { return nullptr; }
bool JSNIIsNumber(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewNumber(JSNIEnv*, double) { return nullptr; }
double JSNIToCDouble(JSNIEnv*, JSValueRef) { return 0.0; }
bool JSNIIsSymbol(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewSymbol(JSNIEnv*, JSValueRef) { return nullptr; }
bool JSNIIsString(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewStringFromUtf8(JSNIEnv*, const char*, size_t) { return nullptr; }
size_t JSNIGetStringUtf8Length(JSNIEnv*, JSValueRef) { return 0; } 
size_t JSNIGetStringUtf8Chars(JSNIEnv*, JSValueRef, char*, size_t) { return 0; }
bool JSNIIsObject(JSNIEnv*, JSValueRef) { return false; }
bool JSNIIsEmpty(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewObject(JSNIEnv*) { return nullptr; }
bool JSNIHasProperty(JSNIEnv*, JSValueRef, const char*) { return false; }
JSValueRef JSNIGetProperty(JSNIEnv*, JSValueRef, const char*) { return nullptr; }
bool JSNISetProperty(JSNIEnv*, JSValueRef, const char*, JSValueRef) { return false; }
bool JSNIDefineProperty(JSNIEnv*, JSValueRef, const char*, const JSNIPropertyDescriptor) { return false; }
bool JSNIDeleteProperty(JSNIEnv*, JSValueRef, const char*) { return false; }
JSValueRef JSNIGetPrototype(JSNIEnv*, JSValueRef) { return nullptr; }
JSValueRef JSNINewObjectWithInternalField(JSNIEnv*, int) { return nullptr; }
int JSNIInternalFieldCount(JSNIEnv*, JSValueRef) { return 0; }
void JSNISetInternalField(JSNIEnv*, JSValueRef, int, void*) {}
void* JSNIGetInternalField(JSNIEnv*, JSValueRef, int) { return nullptr; }
bool JSNIIsFunction(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewFunction(JSNIEnv*, JSNICallback) { return nullptr; }
JSValueRef JSNICallFunction(JSNIEnv*, JSValueRef, JSValueRef, int, JSValueRef*) { return nullptr; }
bool JSNIIsArray(JSNIEnv*, JSValueRef) { return false; }
size_t JSNIGetArrayLength(JSNIEnv*, JSValueRef) { return 0; }
JSValueRef JSNINewArray(JSNIEnv*, size_t) { return nullptr; }
JSValueRef JSNIGetArrayElement(JSNIEnv*, JSValueRef, size_t) { return nullptr; }
void JSNISetArrayElement(JSNIEnv*, JSValueRef, size_t, JSValueRef) {}
bool JSNIIsTypedArray(JSNIEnv*, JSValueRef) { return false; }
JSValueRef JSNINewTypedArray(JSNIEnv*, JsTypedArrayType, void*, size_t) { return nullptr; }
JsTypedArrayType JSNIGetTypedArrayType(JSNIEnv*, JSValueRef) { return JsArrayTypeNone; }
void* JSNIGetTypedArrayData(JSNIEnv*, JSValueRef) { return nullptr; }
size_t JSNIGetTypedArrayLength(JSNIEnv*, JSValueRef) { return 0; }
void JSNIPushLocalScope(JSNIEnv*) {}
void JSNIPopLocalScope(JSNIEnv*) {}
void JSNIPushEscapableLocalScope(JSNIEnv*) {}
JSValueRef JSNIPopEscapableLocalScope(JSNIEnv*, JSValueRef) { return nullptr; }
JSGlobalValueRef JSNINewGlobalValue(JSNIEnv*, JSValueRef) { return nullptr; }
void JSNIDeleteGlobalValue(JSNIEnv*, JSGlobalValueRef) {}
size_t JSNIAcquireGlobalValue(JSNIEnv*, JSGlobalValueRef) { return 0; }
size_t JSNIReleaseGlobalValue(JSNIEnv*, JSGlobalValueRef) { return 0; }
JSValueRef JSNIGetGlobalValue(JSNIEnv*, JSGlobalValueRef) { return nullptr; }
void JSNISetGCCallback(JSNIEnv*, JSGlobalValueRef, void*, JSNIGCCallback) {}
void JSNIThrowErrorException(JSNIEnv*, const char*) {}
void JSNIThrowTypeErrorException(JSNIEnv*, const char*) {}
void JSNIThrowRangeErrorException(JSNIEnv*, const char*) {}
JSNIErrorInfo JSNIGetLastErrorInfo(JSNIEnv*) { return {nullptr, JSNIOK}; }
bool JSNIHasException(JSNIEnv*) { return false; }
void JSNIClearException(JSNIEnv*) {}
