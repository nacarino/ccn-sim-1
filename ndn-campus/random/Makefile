CXX=g++
RM=rm -f
LDLIBS=-lboost_program_options

CSGSRCS=content-size-generator.cc
CSGOBJS=$(subst .cc,.o,$(CSGSRCS))

URLSRCS=url-generator.cc
URLOBJS=$(subst .cc,.o,$(URLSRCS))

POSSRCS=position-generator.cc
POSOBJS=$(subst .cc,.o,$(POSSRCS))

SRCS=$(CSGSRCS) $(URLSRCS) $(POSSRCS)
OBJS=$(CSGOBJS) $(URLOBJS) $(POSOBJS)

all: content-size-generator url-generator position-generator

content-size-generator: $(CSGOBJS)
	g++ -o content-size-generator $(CSGOBJS) $(LDLIBS) 

url-generator: $(URLOBJS)
	g++ -o url-generator $(URLOBJS) $(LDLIBS) 

position-generator: $(POSOBJS)
	g++ -o position-generator $(POSOBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) content-size-generator
	$(RM) url-generator
	$(RM) position-generator

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
