SET(SOURCE
    jsobject.cc
    jsprimitive.cc
    jsvalue.cc
)

SET(HEADERS
    apply.h
    jsarray.h
    jscallback.h
    jsconstructor.h
    jsexception.h
    jsfunction.h
    jsni.h
    jsnipp.h
    jsobject.h
    jsprimitive.h
    jsproperty.h
    jstypedarray.h
    jsvalue.h
)

#include_directories(.)

add_library(jsnipp STATIC ${SOURCE} ${HEADERS})
set_property(TARGET jsnipp PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET jsnipp PROPERTY VISIBILITY_INLINES_HIDDEN TRUE)

#install (TARGETS jsnipp DESTINATION lib)
#install (FILES ${HEADERS} DESTINATION include/jsnipp)



include_directories(.)
add_library(jsnitest MODULE test.cc)
target_link_libraries(jsnitest jsnipp)
#install (TARGETS jsnitest DESTINATION bin)
