load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)
libSrcMkFile := $(abspath $(lastword $(MAKEFILE_LIST)))
$(dwm_aliasfn my,dwm_my)
$(dwm_myns lib)
$(my mydir := $(abspath $(dir $(libSrcMkFile))))

$(dwm_include $(abspath $(my mydir)/../../Makefile.vars))

shlib_version := $(shell $(my mydir)/../../getvers.sh -s)

$(dwm_setltlibvars lib,../lib/libDwmMclog.la,../obj,deps,.+\.(cc|lex|y)$$)

$(my Link       := ${LIBTOOL} --mode=link --tag=CXX $(CXX))
$(my Incs       := -I$(abspath $(my mydir)/../include) -I. ${DWMINCS})
$(my CxxFlags   := ${CXXFLAGS} ${PTHREADCXXFLAGS} $(my Incs))
$(my Compile    := $(LIBTOOLCOMPILE) $(my CxxFlags))

$(eval TARGETS          $(dwm_ifcwd :=,+=) $(my lib.Lib))
$(eval CLEANTARGETS     $(dwm_ifcwd :=,+=) $(my lib.Clean))
$(eval LTCLEANTARGETS   $(dwm_ifcwd :=,+=) $(my lib.LtClean))
$(eval DISTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my lib.Deps))
$(eval TARTARGETS       $(dwm_ifcwd :=,+=) ${TARDIR}/lib/libDwmMclog.la)

$(dwm_include $(abspath $(my mydir)/../../Makefile.rules))

#  dependency rule
$(eval $(dwm_cppdeps $(my mydir)/deps,\
$(abspath $(my mydir)/../obj),$(my mydir)/%.cc,\
${CXX} -MM $(my CxxFlags) $<,.o .lo))

#  only include dependency makefiles if target is not a 'clean' target
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),cleandeps)
$(dwm_include $(my Deps))
endif
endif
endif

$(my lib.Lib):: $(my lib.SharedObjs)
	@dwmgmk_quiet "creating $(dwm_relpwd $@)"\
	 $(LINK) -o $@ $(my lib.SharedObjs) -rpath ${INSTALLPREFIX}/lib \
	  -version-info ${shlib_version} ${DWMLIBS}

$(my ObjDir)/%.lo: $(my mydir)/%.cc $(my mydir)/deps/%_deps
	@dwmgmk_quiet "compiling $(dwm_relpwd $<)" \
	 $(my lib.Compile) -c $< -o $@

$(my lib.mydir)/%Parse.cc $(my lib.mydir)/%Parse.hh &: $(my lib.mydir)/%Parse.y
	@dwmgmk_quiet "generating parser from $(dwm_relpwd $<)" \
	 bison -d -o $(patsubst %.hh,%.cc,$@) $<

$(my lib.mydir)/%Lex.cc: $(my lib.mydir)/%Lex.lex $(my lib.mydir)/%Parse.hh
	@dwmgmk_quiet "generating lexer from $(dwm_relpwd $<)" \
	 flex -o $@ $<

${TARDIR}/lib/%.la: $(my lib.Lib)
	@dwmgmk_quiet "creating $(dwm_relpwd $@)" ${INSTALL} -c $< $@
