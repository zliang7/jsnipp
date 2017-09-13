#include <cassert>
#include <type_traits>

#include <jsni.h>

#include "jsobject.h"

namespace jsnipp {

// TODO: share a base with JSArray
template<typename T, bool clamped = false>
class JSTypedArray final : public JSObject {
public:
    JSTypedArray(): JSObject(env_->NewTypedArray(array_type(), NULL, 0)) {
        static_assert(std::is_arithmetic<T>::value, "");
    }
    JSTypedArray(JsValue jsval): JSObject(jsval) {
        assert(is_typedarray() &&
               env_->GetTypedArrayType(jsval_) == array_type());
    }

    size_t length() const {
        return env_->ToDouble(env_->GetProperty(jsval_, "length"));
    }
    size_t byteLength() const {
        return env_->ToDouble(env_->GetProperty(jsval_, "byteLength"));
    }
    T* buffer() const {
        return reinterpret_cast<T*>(env_->GetTypedArrayData(jsval_));
    }

private:
    static constexpr JsTypedArrayType array_type();
};

template<typename T, bool clamped>
constexpr JsTypedArrayType JSTypedArray<T, clamped>::array_type() {
/*  if (std::is_same<T, int8_t>::value)
        return JsArrayTypeInt8;
    else if (std::is_same<T, uint8_t>::value)
        return clamped ? JsArrayTypeUint8Clamped : JsArrayTypeUint8;
    else if  (std::is_same<T, int16_t>::value)
        return JsArrayTypeInt16;
    else if  (std::is_same<T, uint16_t>::value)
        return JsArrayTypeUint16;
    else if  (std::is_same<T, int32_t>::value)
        return JsArrayTypeInt32;
    else if  (std::is_same<T, uint32_t>::value)
        return JsArrayTypeUint32;
    else if  (std::is_same<T, float>::value)
        return JsArrayTypeFloat32;
    else if  (std::is_same<T, double>::value)
        return JsArrayTypeFloat64;
    else
        return JsArrayTypeNone;*/
    return std::is_same<T, int8_t>::value   ? JsArrayTypeInt8 : (
           std::is_same<T, uint8_t>::value  ? (clamped ? JsArrayTypeUint8Clamped : JsArrayTypeUint8): (
           std::is_same<T, int16_t>::value  ? JsArrayTypeInt16 : (
           std::is_same<T, uint16_t>::value ? JsArrayTypeUint16 : (
           std::is_same<T, int32_t>::value  ? JsArrayTypeInt32 : (
           std::is_same<T, uint32_t>::value ? JsArrayTypeUint32 : (
           std::is_same<T, float>::value    ? JsArrayTypeFloat32 : (
           std::is_same<T, double>::value   ? JsArrayTypeFloat64 : (
           JsArrayTypeNone))))))));
}

}
