// JavaScript Native Interface Release License.
//
// Copyright (c) 2015-2016 Alibaba Group. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Alibaba Group nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _JSNI_H
#define _JSNI_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// JS type.
typedef struct _JsValue* JsValue;

// JSNI interface extended.
struct _JSNIEnv;

typedef const struct JSNINativeInterface JSNIEnv;


// Callback helper types.
typedef void (*JSNIGCCallback)(JSNIEnv* env, void* info);

typedef struct _CallbackInfo* CallbackInfo;
typedef void (*NativeFunctionCallback)(JSNIEnv* env, const CallbackInfo info);

typedef struct _JsGlobalValue* JsGlobalValue;

// Type array.
typedef enum _JsTypedArrayType {
  JsArrayTypeNone,
  JsArrayTypeInt8,
  JsArrayTypeUint8,
  JsArrayTypeUint8Clamped,
  JsArrayTypeInt16,
  JsArrayTypeUint16,
  JsArrayTypeInt32,
  JsArrayTypeUint32,
  JsArrayTypeFloat32,
  JsArrayTypeFloat64
} JsTypedArrayType;

// Table of interface function pointers.
struct JSNINativeInterface {
  void* reserved1;
  void* reserved2;
  void* reserved3;
  void* reserved4;
  void* reserved5;

  // \summary:
  //    Returns the version of the JSNI.
  // \param:
  //    None.
  // \returns:
  //    Returns the version of the JSNI.
  int (*GetVersion)();

  // \summary:
  //    Registers the native method.
  // \param:
  //    recv: the method receiver. The developer may not concern about it.
  //    name: a function name.
  //    callback: a native function to be registered.
  // \returns:
  //    Returns true on success.
  bool (*RegisterMethod)(const JsValue recv, const char* name,
                         NativeFunctionCallback callback);
  // \summary:
  //    Returns the number of arguments from native callback info.
  // \param:
  //    info: the callback info.
  // \returns:
  //    Returns the number of arguments.
  int (*GetArgsLength)(CallbackInfo info);

  // \summary:
  //    Returns the argument from native callback info.
  // \param:
  //    info: the callback info.
  //    id: the index of arguments.
  // \returns:
  //    Returns the argument.
  JsValue (*GetArg)(CallbackInfo info, int id);

  // \summary:
  //    Returns "this" JavaScript object of this native function.
  // \param:
  //    info: the callback info.
  // \returns:
  //    Returns "this" JavaScript object of this native function.
  JsValue (*GetThis)(CallbackInfo info);

  // \summary:
  //    Sets the JavaScript return value of this native function.
  // \param:
  //    info: the callback info.
  //    val: the JavaScript value to return to JavaScript.
  // \returns:
  //    None.
  void (*SetReturnValue)(CallbackInfo info, JsValue val);

  // Primitive Operations

  // \summary:
  //    Tests whether the JavaScript value is undefined.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if the value is undefined.
  bool (*IsUndefined)(JsValue val);

  // \summary:
  //    Constructs a new Undefined JavaScript value.
  // \param:
  //    None.
  // \returns:
  //    Returns an Undefined JavaScript value.
  JsValue (*NewUndefined)();

  // \summary:
  //    Tests whether the JavaScript value is Null.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if the value is Null.
  bool (*IsNull)(JsValue val);

  // \summary:
  //    Constructs a new Null JavaScript value.
  // \param:
  //    None.
  // \returns:
  //    Returns a Null JavaScript value.
  JsValue (*NewNull)();


  // \summary:
  //    Tests whether the JavaScript value is Boolean.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if the value is Boolean.
  bool (*IsBoolean)(JsValue val);

  // \summary:
  //    Converts the JavaScript value to bool.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns the bool value.
  bool (*ToBool)(JsValue val);

  // \summary:
  //    Constructs a new Boolean JavaScript value.
  // \param:
  //    val: a bool value.
  // \returns:
  //    Returns a Boolean JavaScript value.
  JsValue (*NewBoolean)(bool val);

  // \summary:
  //    Tests whether the JavaScript value is Number.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if the value is Number.
  bool (*IsNumber)(JsValue val);

  // \summary:
  //    Constructs a new Number JavaScript value.
  // \param:
  //    val: a double value.
  // \returns:
  //    Returns a Number JavaScript value.
  JsValue (*NewNumber)(double val);

  // \summary:
  //    Converts the JavaScript value to double.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns the double value.
  double (*ToDouble)(JsValue val);


  // \summary:
  //    Tests whether the JavaScript value is String.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if the value is String.
  bool (*IsString)(JsValue val);

  // \summary:
  //    Constructs a new String value from an array of characters
  //    in UTF-8 encoding.
  // \param:
  //    src: the pointer to a UTF-8 string.
  // \returns:
  //    Returns a String value, or NULL if the string can not be constructed.
  JsValue (*NewStringFromUtf8)(const char* src, int len);

  // \summary:
  //    Returns the length in bytes of the UTF-8 representation of a string.
  // \param:
  //    string: a JavaScript string value.
  // \returns:
  //    Returns the UTF-8 length of the string.
  size_t (*GetStringUtf8Length)(JsValue string);

  // \summary:
  //    Returns a pointer to an array of bytes representing the string in UTF-8
  //    encoding. This array must be released by ReleaseStringUtf8Chars() to
  //    avoid memory leak.
  // \param:
  //    string: a JavaScript string value.
  // \returns:
  //    Returns a pointer to a UTF-8 string, or NULL if the operation fails.
  char* (*GetStringUtf8Chars)(JsValue string);

  // \summary:
  //    Informs the VM that the native code no longer needs access to str. The
  //    str argument is a pointer derived from string using GetStringUtf8Chars().
  // \param:
  //    string: a JavaScript string value.
  //    str: a pointer to a UTF-8 string.
  // \returns:
  //    NONE.
  void (*ReleaseStringUtf8Chars)(JsValue string, char* str);

  //Object Operations

  // \summary:
  //    Tests whether a JavaScript value is a JavaScript object.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if val is a JavaScript object.
  bool (*IsObject)(JsValue val);

  // \summary:
  //    Tests whether a JavaScript value is empty.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if val is empty.
  bool (*IsEmpty)(JsValue val);

  // \summary:
  //    Constructs a JavaScript object.
  // \param:
  //    None.
  // \returns:
  //    Returns a JavaScript object.
  JsValue (*NewObject)();

  // \summary:
  //    Constructs a JavaScript object with hidden internal field.
  // \param:
  //    count: a number of hidden fields.
  // \returns:
  //    Returns a JavaScript object.
  JsValue (*NewObjectWithHiddenField)(int count);

  // \summary:
  //    Tests whether a JavaScript object has a property named name.
  // \param:
  //    object: a JavaScript object.
  //    name: a property name.
  // \returns:
  //    Returns true if object has property named name.
  bool (*HasProperty)(JsValue object, const char* name);

  // \summary:
  //    Returns the property of the JavaScript object.
  // \param:
  //    object: a JavaScript object.
  //    name: a property name.
  // \returns:
  //    Returns the property of the JavaScript object.
  JsValue (*GetProperty)(JsValue object, const char* name);

  // \summary:
  //    Sets a property of a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  //    name: a property name.
  //    property: a JavaScript value.
  // \returns:
  //    Returns true if the operation succeeds.
  bool (*SetProperty)(JsValue object, const char* name, JsValue property);

  // \summary:
  //    Deletes the property of a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  //    name: a property name.
  // \returns:
  //    Returns true if the operation succeeds.
  bool (*DeleteProperty)(JsValue object, const char* name);

  // \summary:
  //    Returns a prototype of a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  // \returns:
  //    Returns a JavaScript value.
  JsValue (*GetPrototype)(JsValue object);

  // \summary:
  //    Gets the number of the hidden internal field fo a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  // \returns:
  //    Returns the number of the hidden internal field.
  int (*HiddenFieldCount)(JsValue object);

  // \summary:
  //    Sets a hidden internal field of a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  //    index: index of a hidden internal field.
  //    field: a hidden internal field.
  // \returns:
  //    None.
  void (*SetHiddenField)(JsValue object, int index, void* field);

  // \summary:
  //    Gets a hidden internal field of a JavaScript object.
  // \param:
  //    object: a JavaScript object.
  //    index: index of a hidden internal field.
  // \returns:
  //    a hidden internal field.
  void* (*GetHiddenField)(JsValue object, int index);

  // \summary:
  //    Tests whether a JavaScript value is Function.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if val is Function.
  bool (*IsFunction)(JsValue val);

  // \summary:
  //    Constructs a JavaScript function with nativeFunc.
  // \param:
  //    nativeFunc: a native function.
  // \returns:
  //    Returns a JavaScript function.
  JsValue (*NewFunction)(NativeFunctionCallback nativeFunc);

  // \summary:
  //    Calls a JavaScript function.
  // \param:
  //    func: a JavaScript funciton.
  //    recv: the receiver the func belongs to.
  //    argc: the arguments number.
  //    argv: a pointer to an array of JavaScript value.
  // \returns:
  //    Returns the JavaScript value returned from calling func.
  JsValue (*CallFunction)(JsValue func, JsValue recv, int argc,
               JsValue* argv);


  // \summary:
  //    Tests whether a JavaScript value is Array.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if val is Array.
  bool (*IsArray)(JsValue val);

  // \summary:
  //    Returns the number of elements in the array.
  // \param:
  //    array: a JavaScript array.
  // \returns:
  //    Returns the length of the array.
  size_t (*GetArrayLength)(JsValue array);

  // \summary:
  //    Constructs a JavaScript array with initial length: initial_length.
  // \param:
  //    initial_length: initial array size.
  // \returns:
  //    Returns a JavaScript array object, or NULL if the operation fails.
  JsValue (*NewArray)(size_t initial_length);

  // \summary:
  //    Returns an element of a JavaScript array.
  // \param:
  //    array: a JavaScript array.
  //    index: array index.
  // \returns:
  //    Returns a JavaScript value.
  JsValue (*GetArrayElement)(JsValue array, size_t index);

  // \summary:
  //    Sets an element of a JavaScript array.
  // \param:
  //    array: a JavaScript array.
  //    index: a array index.
  //    value: a new value.
  // \returns:
  //    None.
  void (*SetArrayElement)(JsValue array, size_t index, JsValue value);

  // \summary:
  //    Tests whether a JavaScript value is TypedArray.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns true if val is TypedArray.
  bool (*IsTypedArray)(JsValue val);

  // \summary:
  //    Constructs a JavaScript TypedArray object.
  // \param:
  //    type: the type of the array.
  //    data: the pointer to the data buffer of the array.
  //    length: the length of the array.
  // \returns:
  //    Returns a JavaScript TypedArray object.
  JsValue (*NewTypedArray)(JsTypedArrayType type, void* data, size_t length);

  // \summary:
  //    Returns the type of the JavaScript TypedArray value.
  // \param:
  //    typed_array: a JavaScript TypedArray value.
  // \returns:
  //    Returns the type of the JavaScript TypedArray value.
  JsTypedArrayType (*GetTypedArrayType)(JsValue typed_array);

  // \summary:
  //    Returns the pointer to the buffer of TypedArray data.
  // \param:
  //    typed_array: a JavaScript TypedArray value.
  // \returns:
  //    Returns the pointer to the buffer of TypedArray data.
  void* (*GetTypedArrayData)(JsValue typed_array);

  //Reference

  // \summary:
  //    Creates a local reference scope, and then all local references will
  //    be allocated within this reference scope until the reference scope
  //    is deleted using PoplocalScope() or another local reference scope
  //    is created.
  // \param:
  //    None.
  // \returns:
  //    None.
  void (*PushLocalScope)();

  // \summary:
  //    Pops off the current local reference scope, frees all the local
  //    references in the local reference scope, and returns a local reference
  //    in the previous local scope for the given val JavaScript value.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns a JavaScript value.
  JsValue (*PopLocalScope)(JsValue val);

  // \summary:
  //    Creates a new global reference to the JavaScript value referred to by
  //    the val argument. The global value must be explicitly disposed of by
  //    calling DeleteGlobalValue(), except that the global value is set to
  //    weak by calling SetWeakGCCallback(). The global value will be alive
  //    untile calling DeleteGlobalValue() to dispose it.
  // \param:
  //    val: a JavaScript value.
  // \returns:
  //    Returns a global value.
  JsGlobalValue (*NewGlobalValue)(JsValue val);

  // \summary:
  //    Deletes the global reference pointed by val.
  // \param:
  //    val: a JsGlobalValue value.
  // \returns:
  //    None.
  void (*DeleteGlobalValue)(JsGlobalValue val);

  // \summary:
  //    Returns a local JsValue value from a JsGlobalValue value.
  // \param:
  //    val: a JsGlobalValue value.
  // \returns:
  //    Returns a local JsValue value.
  JsValue (*GetGlobalValue)(JsGlobalValue val);

  // \summary:
  //    Sets a callback which will be called when the JavaScript value pointed by val
  //    is freed. The developer can pass an argument to callback by args.
  // \param:
  //    val: a JavaScript value.
  //    args: a pointer to an argument passed to callback.
  //    callback: a function callback.
  // \returns:
  //    None.
  void (*SetGCCallback)(JsValue val, void* args, JSNIGCCallback callback);

  // \summary:
  //    Makes the JsGlobaValue val weak, and sets a callback which will be
  //    called when the JavaScript value pointed by val is freed. A weak global
  //    reference allows the underlying JavaScript value to be garbage
  //    collected. The developer can pass an argument to callback by args.
  // \param:
  //    val: a JsGlobalValue value.
  //    args: a pointer to an argument passed to callback.
  //    callback: a function callback.
  // \returns:
  //    None.
  void (*SetWeakGCCallback)(JsGlobalValue val, void* args, JSNIGCCallback callback);

  //Exception

  // \summary:
  //    Constructs an error object with the message specified by errmsg
  //    and causes that error to be thrown. It throws a JavaScript Exception.
  // \param:
  //    errmsg: an error message.
  // \returns:
  //    None.
  void (*ThrowErrorException)(const char* errmsg);

  // \summary:
  //    Constructs an type error object with the message specified by errmsg
  //    and causes that type error to be thrown. It throws a JavaScript Exception.
  // \param:
  //    errmsg: an error message.
  // \returns:
  //    None.
  void (*ThrowTypeErrorException)(const char* errmsg);

  // \summary:
  //    Constructs an range error object with the message specified by errmsg
  //    and causes that type error to be thrown. It throws a JavaScript Exception.
  // \param:
  //    errmsg: an error message.
  // \returns:
  //    None.
  void (*ThrowRangeErrorException)(const char* errmsg);

  // \summary:
  //    Tests whether there is error occured during pervious JSNI call. After
  //    calling ErrorCheck(), if there is error occured, the error will be
  //    cleared.
  // \param:
  //    None.
  // \returns:
  //    Returns true if there is error occured during previous JSNI call.
  bool (*ErrorCheck)();

};


/*
 * Extended jsni_env which contains functions table pointer.
 */
struct _JSNIEnv {
  const struct JSNINativeInterface* functions;
};

// JSNI Versions.
#define JSNI_VERSION_1_0 0x00010000
#define JSNI_VERSION_1_1 0x00010001

#if defined(__cplusplus)
extern "C" {
#endif
/*
 * This function is called by JSNI, not part of JSNI.
 */
int JSNI_Init(JSNIEnv* env, JsValue exports);

#if defined(__cplusplus)
}
#endif


#endif
