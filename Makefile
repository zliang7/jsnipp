CXXFLAGS = -m32 -std=c++11 -g -I jsni -I .

ifeq ($(shell uname),Darwin)
CXXFLAGS += -Wno-deprecated-declarations
endif

SOURCE = test/test.cc test/jsni_dummy.cc
OBJECT = $(SOURCE:.cc=.o)
DEPEND = $(SOURCE:.cc=.d)
MODULE = test/jsnitest.so
DEPEND = test/depends

$(MODULE): $(OBJECT)
	$(CXX) -m32 -shared -o $@ $^

clean:
	rm -f $(OBJECT) $(DEPEND) $(MODULE) $(DEPEND)

#%.d: %.cc
#	$(CXX) -MM $(CXXFLAGS) -o $@ $<

#.SUFFIXEX: clean

#-include $(DEPEND)

# Generate dependance file
$(DEPEND): $(SOURCE)
	$(CXX) $(CXXFLAGS) -MM $^ > $@
-include $(DEPEND)
