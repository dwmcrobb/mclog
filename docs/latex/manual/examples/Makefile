load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)

exampleDir := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
$(dwm_myns examples)
$(dwm_aliasfn my,dwm_my)
$(dwm_include $(abspath $(exampleDir)/../../../../classes/src/Makefile))

$(my Link     := ${LIBTOOL} --mode=link --tag=CXX ${CXX})
$(my ObjNames := $(patsubst %.cc,%.o,$(dwm_files $(exampleDir),.+\.cc)))
$(my Objs     := $(patsubst %.o,$(exampleDir)/%.o,$(my ObjNames)))
$(my ObjDeps  := $(patsubst %.o,$(exampleDir)/deps/%_deps,$(my ObjNames)))
$(my Exes     := $(patsubst %.o,%,$(my Objs)))
$(my Incs     := ${DWMINCS} -I$(abspath $(exampleDir)/../../../../classes/include))
$(my Libs     := ${DWMLIBS})

$(eval TARGETS $(dwm_ifcwd :=,+=) $(my Exes))
$(eval DEPSTARGETS $(dwm_ifcwd :=,+=) $(my ObjDeps))
$(eval CLEANTARGETS $(dwm_ifcwd :=,+=) $(my Objs))
$(eval LTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my Exes))
$(eval DISTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my ObjDeps))

$(dwm_include $(abspath $(exampleDir)/../../../../Makefile.rules))

#  generate dependency rule
$(eval $(dwm_cppdeps $(exampleDir)/deps,\
$(exampleDir),$(exampleDir)/%.cc,\
${CXX} -MM ${CXXFLAGS} $(my Incs) -c $<))

#  only include dependency makefiles if target is not a 'clean' target
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),cleandeps)
$(dwm_include $(my ObjDeps))
endif
endif
endif

$(exampleDir)/%.o: $(exampleDir)/%.cc $(exampleDir)/deps/%_deps
	@dwmgmk_quiet "compiling $(dwm_relpwd $<)" \
	  ${CXX} ${CXXFLAGS} ${PTHREADCXXFLAGS} $(my examples.Incs) -c $< -o $@

$(exampleDir)/Example%: $(exampleDir)/Example%.o \
  $(abspath $(exampleDir)/../../../../classes/lib/libDwmMclog.la)
	@dwmgmk_quiet "linking $(dwm_relpwd $@)" \
	  $(my examples.Link) ${LDFLAGS} -o $@ $^ $(my examples.Libs)
