CXXFLAGS = -m32 -std=c++11 -g -I jsni

ifeq ($(shell uname),Darwin)
CXXFLAGS += -Wno-deprecated-declarations
endif

SOURCE = jsobject.cc  jsprimitive.cc  jsvalue.cc test.cc
OBJECT = $(SOURCE:.cc=.o)
DEPEND = $(SOURCE:.cc=.d)

jsnitest.so: $(OBJECT)
	$(CXX) -m32 -shared -o $@ $^

clean:
	rm -f $(OBJECT) $(DEPEND) jsnitest.so depends

#%.d: %.cc
#	$(CXX) -MM $(CXXFLAGS) -o $@ $<

#.SUFFIXEX: clean

#-include $(DEPEND)

# Generate dependance file
depends: $(SOURCE)
	$(CXX) $(CXXFLAGS) -MM $^ > $@
-include depends
