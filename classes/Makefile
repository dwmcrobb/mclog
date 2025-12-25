load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)
classesDir := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

$(dwm_include $(classesDir)/tests/Makefile)
$(dwm_include $(classesDir)/include/Makefile)

$(dwm_myns classes)
$(dwm_aliasfn my,dwm_my)
$(dwm_aliasfn ifcwd,dwm_ifcwd)

$(eval TARGETS $(ifcwd :=,?=) $(my lib.Lib,tests.Exes))
$(eval CLEANTARGETS $(ifcwd :=,?=) $(my lib.Clean))
$(eval LTCLEANTARGETS $(ifcwd :=,?=) $(my lib.LtClean,tests.Exes))
$(eval DISTCLEANTARGETS $(ifcwd :=,?=) $(my lib.Deps) $(libIncTarPrepHdrs))
$(eval TARTARGETS $(ifcwd :=,?=) ${TARDIR}/lib/libDwmMclog.la $(libIncTarPrepHdrs))
