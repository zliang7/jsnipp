#include "jsobject.h"
#include "jsfunction.h"

using namespace jsnipp;

/* https://stackoverflow.com/a/28411055/2186063
template <typename T, std::size_t... Indices>
auto vectorToTupleHelper(const std::vector<T>& v, std::index_sequence<Indices...>) {
    return std::make_tuple(v[Indices]...);
}

template <std::size_t N, typename T>
auto vectorToTuple(const std::vector<T>& v) {
    assert(v.size() >= N);
    return vectorToTupleHelper(v, std::make_index_sequence<N>());
}
*/

/* function traits
https://stackoverflow.com/a/7943765/2186063
https://github.com/kennytm/utils/blob/master/traits.hpp
http://www.boost.org/doc/libs/1_61_0/libs/type_traits/doc/html/boost_typetraits/reference/function_traits.html
https://functionalcpp.wordpress.com/2013/08/05/function-traits/
*/

template <class... Args>
using XXXX = JSValue (*)(JSObject, Args...);
template <XXXX<const CallbackInfo> function>
class JSNativeFunction1: public JSFunction {
public:
    JSNativeFunction1():
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSObject self = env->GetThis(info);
            JsValue result = (*function)(self, info);
            env->SetReturnValue(info, result);
        }){}
};



template <typename> struct member_function_traits;

template <typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...)> {
    typedef Return return_type;
    typedef Object instance_type;
    typedef Object & instance_reference;

    // Can mess with Args... if you need to, for example:
    static constexpr size_t argument_count = sizeof...(Args);
};
// If you intend to support const member functions you need another specialization.
template <typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...) const> {
    typedef Return return_type;
    typedef Object instance_type;
    typedef Object const & instance_reference;

    // Can mess with Args... if you need to, for example:
    static constexpr size_t argument_count = sizeof...(Args);
};
