AC_INIT([mclog],m4_esyscmd_s([./getvers.sh -v]),[dwmcrobb@me.com],[mclog],[http://www.mcplex.net])

AC_PROG_CC
AC_PROG_CXX
AC_CANONICAL_TARGET

dnl  ###  Read some generally useful macros.
builtin(include, ./dwm.m4)  dnl
builtin(include, ax_check_compile_flag.m4) dnl

dnl  Set variables for building shared library
DWM_SET_CXX_SHARED_FLAGS

dnl  Set variables for pthreads
DWM_SET_PTHREADFLAGS

OSLIBS=""
KVMLIB="-lkvm"
LIBTOOL="libtool"

case $host_os in
  freebsd[[89]]*)
    CXXFLAGS="-O2"
    LDFLAGS="-static"
    ;;
  freebsd1[[01234]]*)
    CXXFLAGS="-O2"
    LDFLAGS="-static"
    ;;
  linux*)
    CXXFLAGS="-O2"
    KVMLIB=""
    LDFLAGS="$LDFLAGS"
    DEBARCH=`dpkg --print-architecture`
    AC_SUBST(DEBARCH)
    ;;
  darwin*)
    CC="clang++"
    CXX="clang++"
    CXXFLAGS="${CXXFLAGS} -O2 -stdlib=libc++"
    KVMLIB=""
    LDFLAGS="${LDFLAGS} -O2 -stdlib=libc++ -std=c++23"
    LIBTOOL="glibtool"
    ;;
esac

echo "host_os ${host_os}"

dnl  Check for C++23
DWM_CHECK_CPLUSPLUS_23

DWM_CHECK_NEED_LIBATOMIC

AC_SUBST(KVMLIB)
AC_SUBST(LIBTOOL)
AC_SUBST(OSLIBS)

DWM_REQUIRES_DWMPKG(libDwm,0.9.61)
DWM_REQUIRES_DWMPKG(libDwmCredence,0.2.6)
DWM_REQUIRES_DWMPKG(dwmgmk,0.0.0)

DWMDIR=`pkg-config --variable=prefix libDwm`
DWMINCS=`pkg-config --cflags-only-I libDwmCredence`
DWMLIBS=`pkg-config --libs libDwmCredence`
AC_SUBST(DWMINCS)
AC_SUBST(DWMLIBS)
AC_SUBST(DWMDIR)

DWM_CHECK_PKG(dwmwhat,[exit 1])
DWM_CHECK_STD_FORMAT
if [[ ${DWM_HAVE_STD_FORMAT} -eq 0 ]]; then
  DWM_CHECK_PKG(fmt)
  DWM_CHECK_LIBFMT
fi

AC_SUBST(PC_REQ_PKGS)
PKG_EXTINCS=`pkg-config --cflags-only-I ${PC_PKGS}`
PKG_EXTLIBS=`pkg-config --libs ${PC_PKGS}`
EXTINCS=`dwmgmk_uniqleft ${PKG_EXTINCS} ${EXTINCS}`
AC_SUBST(EXTINCS)

DWM_CHECK_BOOSTASIO
DWM_CHECK_NEED_LIBIBVERBS

AC_SUBST(LDFLAGS)

DWM_CHECK_STRTOF()

DWM_PREREQ_PATH(htmlman,share/htmlman,[HTML manpage path])

DWM_SET_PKGVARS(mclog,[mclog-0.0.0])
DWM_GET_TAG([mclog])
DWM_WHAT_STATUS=`./getvers.sh -S`
AC_SUBST(DWM_WHAT_STATUS)

AC_CHECK_PROG([MANDOC], [mandoc], [mandoc], [])

AC_SUBST(CXXFLAGS)

DWM_CHECK_LIBSTDCPPFS

BUILD_DOCS=""
AC_ARG_ENABLE([docs],[AS_HELP_STRING([--enable-docs],[build documentation])],
	      [BUILD_DOCS="yes"], [])
AC_SUBST(BUILD_DOCS)

AC_CONFIG_FILES([Makefile.vars packaging/debcontrol
                 packaging/fbsd_manifest packaging/mclog.pc
		 classes/include/DwmMclogSettings.hh
		 classes/include/DwmMclogVersion.hh
		 etc/macos/net.mcplex.mclogd.plist])
AC_CONFIG_FILES([etc/macos/scripts/postinstall],
		[chmod a+x etc/macos/scripts/postinstall])
AC_CONFIG_FILES([etc/macos/scripts/preinstall],
		[chmod a+x etc/macos/scripts/preinstall])
AC_CONFIG_FILES([etc/freebsd/+POST_INSTALL],
		[chmod a+x etc/freebsd/+POST_INSTALL])
		 
dnl AC_CONFIG_COMMANDS([chmod_macos_scripts],
dnl                    [chmod a+x etc/macos/scripts/postinstall
dnl 		    chmod a+x etc/macos/scripts/preinstall],
dnl 		   [])
AC_OUTPUT
