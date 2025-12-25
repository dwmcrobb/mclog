load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)
mclogMkFile  := $(abspath $(lastword $(MAKEFILE_LIST)))
$(dwm_myns mclog)
$(dwm_aliasfn my,dwm_my)
$(my mydir := $(abspath $(dir $(mclogMkFile))))

$(dwm_include_once $(abspath $(my mydir)/../../classes/Makefile))

$(my CppSrcs  := $(dwm_files $(my mydir),.+\.cc))
$(my Objs     := $(patsubst %.cc,$(my mydir)/%.o,$(my CppSrcs)))
$(my ObjDeps  := $(patsubst %.cc,$(my mydir)/deps/%_deps,$(my CppSrcs)))
$(my Incs     := $(DWMINCS) -I$(my mydir) $(MCLOGINCS))
$(my Targets  := $(my mydir)/mclog)
$(my Clean    := $(my Objs))
$(my LtClean  := $(my mydir)/mclog)

$(eval TARGETS          $(dwm_ifcwd :=,+=) $(my Targets))
$(eval DEPSTARGETS      $(dwm_ifcwd :=,+=) $(my ObjDeps))
$(eval TARTARGETS       $(dwm_ifcwd :=,+=) ${TARDIR}/sbin/mclog)
$(eval CLEANTARGETS     $(dwm_ifcwd :=,+=) $(my Clean))
$(eval LTCLEANTARGETS   $(dwm_ifcwd :=,+=) $(my LtClean))
$(eval DISTCLEANTARGETS $(dwm_ifcwd :=,+=) $(my ObjDeps))

$(dwm_include $(abspath $(my mydir)/../../Makefile.rules))

$(my mydir)/mclog: $(my Objs) $(MCLOGLIB)
	@dwmgmk_quiet "linking $(dwm_relpwd $@)" \
	  $(LINK) -o $@ $^ -rpath ${DWMDIR}/lib $(DWMLIBS)

#  generate dependency rule
$(eval $(dwm_cppdeps $(my mydir)/deps,\
$(my mydir),$(my mydir)/%.cc,\
${CXX} -MM ${CXXFLAGS} $(my Incs) -c $<))

#  only include dependency makefiles if target is not a 'clean' target
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),cleandeps)
include $(my ObjDeps)
endif
endif
endif

$(my mydir)/%.o: $(my mydir)/%.cc $(my mydir)/deps/%_deps
	@dwmgmk_quiet "compiling $(dwm_relpwd $<)" \
	  ${CXX} ${CXXFLAGS} ${PTHREADCXXFLAGS} $(my mclog.Incs) -c $< -o $@

${TARDIR}/sbin/mclog: $(my mydir)/mclog
	${INSTALL} -s -c -m 555 $< $@

