/*
 * {{{ Copyright (C) 2015 The YunOS Project. All rights reserved. }}}
 */
#ifndef YUNOS_BASE_INCLUDE_UTIL_UTIL_H
#define YUNOS_BASE_INCLUDE_UTIL_UTIL_H

#define YUNOS_EXPORT __attribute__ ((visibility ("default")))
#define YUNOS_INTERNAL __attribute__ ((visibility ("hidden")))

#define YUNOS_NO_COPY(CLASS) \
    CLASS(const CLASS&) = delete; \
    void operator=(const CLASS&) = delete;

#define MSEC_PER_SEC 1000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000

/**
 * provide macro definition for classs member and it's getter setter method
 *
 * In the macro definition, the parameter that binding with '#' or "##"
 * will not be replaced during macro expansion.
 * like:
 *     #define NAME 'yunos'
 *     #define TAG(NAME) #NAME
 * TAG(NAME) will get the result of "NAME", rather than "yunos".
 * Thus, the outer macro definition wrapper will be necessary to solve this problem.
 */

// Declare property and getter/setter method for CLASS
#define CLASS_PROPERTY_DECLARE(TYPE, NAME) \
    CLASS_PROPERTY_DECLARE_INTERNAL(TYPE, NAME)

/**
 * Declare property and get/set method for CLASS
 * @param[in] TYPE : type of property-name of CLASS
 * @param[in] NAME : property-name of CLASS
 */
#define CLASS_PROPERTY_DECLARE_INTERNAL(TYPE, NAME) \
    public:                                         \
        inline void set##NAME(const TYPE& value) {  \
            this->m##NAME = value;                  \
        }                                           \
        inline const TYPE& get##NAME() const {      \
            return m##NAME;                         \
        }                                           \
    private:                                        \
        TYPE m##NAME;
// CLASS_PROPERTY_DECLARE

#ifdef __cplusplus
struct uv_handle_s;
namespace yunos {
namespace util {
YUNOS_EXPORT void destroyUvHandle(uv_handle_s* handle);
} // namespace util
} // namespace yunos
#endif

#define WRITE_BOOT_LOG(event) yunosEventWriteStr(42, \
    yunos::String::format("boot_progress, %s %lld", \
                          event.c_str(), \
                          uv_boot_time() / NSEC_PER_MSEC \
                          ).c_str())
#endif // YUNOS_BASE_INCLUDE_UTIL_UTIL_H

