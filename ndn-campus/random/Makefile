CXX=g++
RM=rm -f
LDLIBS=-lboost_program_options

CSGSRCS=content-size-generator.cc
CSGOBJS=$(subst .cc,.o,$(CSGSRCS))

URLSRCS=url-generator.cc
URLOBJS=$(subst .cc,.o,$(URLSRCS))

SRCS=$(CSGSRCS) $(URLSRCS)
OBJS=$(CSGOBJS) $(URLOBJS)

all: content-size-generator url-generator

content-size-generator: $(CSGOBJS)
	g++ -o content-size-generator $(CSGOBJS) $(LDLIBS) 

url-generator: $(URLOBJS)
	g++ -o url-generator $(URLOBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) content-size-generator
	$(RM) url-generator

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
