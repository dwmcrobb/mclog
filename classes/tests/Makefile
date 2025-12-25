load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)
testsDir := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
$(dwm_myns tests)
$(dwm_aliasfn my,dwm_my)
$(dwm_include $(abspath $(testsDir)/../src/Makefile))

$(my Link     := ${LIBTOOL} --mode=link --tag=CXX ${CXX})
$(my ObjNames := $(patsubst %.cc,%.o,$(dwm_files $(testsDir),.+\.cc)))
$(my Objs     := $(patsubst %.o,$(testsDir)/%.o,$(my ObjNames)))
$(my ObjDeps  := $(patsubst %.o,$(testsDir)/deps/%_deps,$(my ObjNames)))
$(my Exes     := $(patsubst %.o,%,$(my Objs)))
$(my Incs     := ${DWMINCS} -I$(abspath $(testsDir)/../include))
$(my Libs     := ${DWMLIBS})

$(eval TARGETS $(dwm_ifcwd :=,+=) $(my Exes))
$(eval DEPSTARGETS $(dwm_ifcwd :=,+=) $(my ObjDeps))
$(eval CLEANTARGETS $(dwm_ifcwd :=,+=) $(my Objs))
$(eval LTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my Exes))
$(eval DISTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my ObjDeps))

$(dwm_include $(abspath $(testsDir)/../../Makefile.rules))

#  generate dependency rule
$(eval $(dwm_cppdeps $(testsDir)/deps,\
$(testsDir),$(testsDir)/%.cc,\
${CXX} -MM ${CXXFLAGS} $(my Incs) -c $<))

#  only include dependency makefiles if target is not a 'clean' target
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),cleandeps)
$(dwm_include $(my ObjDeps))
endif
endif
endif

.PHONY: runtests

$(testsDir)/%.o: $(testsDir)/%.cc $(testsDir)/deps/%_deps
	@dwmgmk_quiet "compiling $(dwm_relpwd $<)" \
	  ${CXX} ${CXXFLAGS} ${PTHREADCXXFLAGS} $(my tests.Incs) -c $< -o $@

$(testsDir)/Test%: $(testsDir)/Test%.o \
  $(abspath $(testsDir)/../lib/libDwmMclog.la)
	@dwmgmk_quiet "linking $(dwm_relpwd $@)" \
	  $(my tests.Link) ${LDFLAGS} -o $@ $^ $(my tests.Libs)

ifeq ($(_classes_tests_Makefile_),)
_classes_tests_Makefile_ := defined
runtests: $(my tests.Exes)
	@ for tp in $(my tests.Exes) ; do \
		printf "%-36s " `basename $$tp` ; \
		out=`$$tp` ; \
		if [ $$? -eq 0 ]; then \
		  printf "%25s\n" "$$out" ; \
		else \
		  printf "\n%s" "$$out" ; \
		fi ; \
	done
endif

