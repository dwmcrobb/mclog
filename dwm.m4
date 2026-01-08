dnl -------------------------------------------------------------------------
dnl  Turns a version triplet string (e.g. '2.1.5') into an integer that can
dnl  be used to compare different versions of my software.
dnl -------------------------------------------------------------------------
define(DWMPKG_TRIPLET_TO_INTEGER,[
  dwm_triple_vers_str=$1
  dwm_triple_vers_maj=${dwm_triple_vers_str%%\.*}
  dwm_triple_vers_rev=${dwm_triple_vers_str##*.}
  dwm_triple_vers_min=${dwm_triple_vers_str%?$dwm_triple_vers_rev}
  dwm_triple_vers_min=${dwm_triple_vers_min##*.}
  $2=$(((dwm_triple_vers_maj * 1000000) + (dwm_triple_vers_min * 1000) + dwm_triple_vers_rev))
])

dnl --------------------------------------------------------------------------
dnl  Checks if there is a pkg-config for $1.  If $2 is given, it is
dnl  interpreted as a required minimum version of $1 and must be an X.Y.Z
dnl  triplet (e.g. '1.0.5').  All of my own software follows this convention.
dnl --------------------------------------------------------------------------
define(DWM_REQUIRES_DWMPKG,[
  if test [$#] -gt 1 ; then
    AC_MSG_CHECKING([for required $1 version >= $2])
  else
    AC_MSG_CHECKING([for required $1])
  fi
  pkg-config --exists $1
  if test $? -eq 0 ; then
    dwm_pkg_full_version=$(pkg-config --modversion $1)
    DWMPKG_TRIPLET_TO_INTEGER([$dwm_pkg_full_version],dwm_pkg_version_installed)
    if test [$#] -gt 1 ; then
      DWMPKG_TRIPLET_TO_INTEGER([$2],dwm_pkg_version_minimum)
      if test ${dwm_pkg_version_installed} -lt ${dwm_pkg_version_minimum} ; then
         AC_MSG_RESULT([need at least version $2 of $1!!!])
	 exit 1
      fi
    fi
    AC_MSG_RESULT([found version ${dwm_pkg_full_version}])
  else
    AC_MSG_RESULT([not found!!!])
    exit 1
  fi
])

dnl --------------------------------------------------------------------------
dnl --------------------------------------------------------------------------
define(DWM_PRESET_PATH,[
  case $1 in
    sbindir)
      if test "$[$1]" = '${exec_prefix}/sbin'; then
        $1=$2
      fi
      ;;
    datadir)
      if test "$[$1]" = '${prefix}/share'; then
        $1=$2
      fi
      ;;
    bindir)
      if test "$[$1]" = '${exec_prefix}/bin'; then
        $1=$2
      fi
      ;;
    libdir)
      if test "$[$1]" = '${exec_prefix}/lib'; then
        $1=$2
      fi
      ;;
    libexecdir)
      if test "$[$1]" = '${exec_prefix}/libexec'; then
        $1=$2
      fi
      ;;
    sysconfdir)
      if test "$[$1]" = '${prefix}/etc'; then
        $1=$2
      fi
      ;;
    sharedstatedir)
      if test "$[$1]" = '${prefix}/com'; then
        $1=$2
      fi
      ;;
    localstatedir)
      if test "$[$1]" = '${prefix}/var'; then
        $1=$2
      fi
      ;;
    infodir)
      if test "$[$1]" = '${prefix}/info'; then
        $1=$2
      fi
      ;;
    mandir)
      if test "$[$1]" = '${prefix}/man'; then
        $1=$2
      fi
      ;;
    srcdir)
      ;;
    includedir)
      if test "$[$1]" = '${prefix}/include'; then
        $1=$2
      fi
      ;;
    *)
      ;;
  esac
  if test -z "$dwm_install_max_name_len" ; then
    dwm_install_max_name_len=`echo $1 | wc -c | awk '{print [$1]}'`
  else
    if test `echo $1 | wc -c` -ge [$dwm_install_max_name_len] ; then
      dwm_install_max_name_len=`echo $1 | wc -c | awk '{print [$1]}'`
    fi
  fi
  dwm_preset_path_names="$dwm_preset_path_names $1"
  dwm_preset_path_values="$dwm_preset_path_values $[$1]"
])


dnl  #############################################################
dnl  ###  Function to add a path for an installation directory.  #
dnl  ###  We'll check/set the variable whose name is passed      #
dnl  ###  as the first argument ($1), which is also a '--with-'  #
dnl  ###  argument for configure.  The second argument should    #
dnl  ###  be the default value for the variable named by $1.     #
dnl  #############################################################
define(DWM_INSTALL_PATH,[
  $1=$2
  AC_ARG_WITH([$1],
              [  --with-$1=value set default \$$1 to value],
              [$1=$withval; /usr/bin/printf "%s set to %s\n" [$1] $withval],
	      [/usr/bin/printf "%s set to %s [(default)]\n" [$1] [$2]]
  )dnl
  AC_SUBST([$1])
  if test -z "$dwm_install_max_name_len" ; then
    dwm_install_max_name_len=`echo [$1] | wc -c | awk '{print [$1]}'`
  else
    if test `echo $[$1] | wc -c` -ge [$dwm_install_max_name_len] ; then
      dwm_install_max_name_len=`echo [$1] | wc -c | awk '{print [$1]}'`
    fi
  fi
  dwm_install_names="$dwm_install_names $1"
  dwm_install_vals="$dwm_install_vals $[$1]"
])dnl

dnl  #############################################################
dnl  ###  Function to add a path for a prerequisite.             #
dnl  ###  The prerequisite is the variable whose name is passed  #
dnl  ###  as the first argument, which is also a '--with-'       #
dnl  ###  argument for configure.  The second argument should    #
dnl  ###  be the default value for the variable named by $1.     #
dnl  #############################################################
define(DWM_PREREQ_PATH,[
  $1=$2
  AC_ARG_WITH([$1],
    [AS_HELP_STRING([--with-$1=value],[set $1 to value ($3)])],
    [$1=$withval; /usr/bin/printf "%s set to %s\n" [$1] $withval],
    [if test -n "$withval" ; then 
       /usr/bin/printf "%s set to %s [(default)]\n" [$1] [$2]
     else
       /usr/bin/printf "%s not set\n" [$1]
     fi
    ]
  )dnl
  AC_SUBST([$1])
  if test -z "$dwm_prereq_max_name_len" ; then
    dwm_prereq_max_name_len=`echo [$1] | wc -c | awk '{print [$1]}'`
  else
    if test `echo $[$1] | wc -c` -ge [$dwm_prereq_max_name_len] ; then
      dwm_prereq_max_name_len=`echo [$1] | wc -c | awk '{print [$1]}'`
    fi
  fi
  dwm_prereq_names="$dwm_prereq_names $1"
  dwm_prereq_vals="$dwm_prereq_vals $[$1]"
])dnl

dnl  #############################################################
dnl  ###  Function to list prereq paths added with               #
dnl  ###  DWM_PREREQ_PATH.                                       #
dnl  #############################################################
define(DWM_LIST_PREREQ_PATHS,[
[
/bin/echo ""
/bin/echo "Prereq paths:"
dwm_name_index=0
for dwm_prereq_name in $dwm_prereq_names ; do
  dwm_val_index=0
  for dwm_prereq_value in $dwm_prereq_vals ; do
    if test "$dwm_name_index" = "$dwm_val_index"; then
      /usr/bin/printf "    %${dwm_prereq_max_name_len}s:  %s\n" $dwm_prereq_name $dwm_prereq_value
    fi
    dwm_val_index=`expr $dwm_val_index + 1`
  done
  dwm_name_index=`expr $dwm_name_index + 1`
done
/bin/echo ""
]
])

dnl  #############################################################
dnl  ###  Function to list the paths added with                  #
dnl  ###  DWM_INSTALL_PATH.                                      #
dnl  #############################################################
define(DWM_LIST_INSTALL_PATHS,[
/bin/echo ""
/bin/echo "Install paths: "
dwm_name_index=0
for dwm_install_name in $dwm_install_names ; do
  dwm_val_index=0
  for dwm_install_value in $dwm_install_vals ; do
    if test "$dwm_name_index" = "$dwm_val_index"; then
      /usr/bin/printf "    %${dwm_install_max_name_len}s:  %s\n" $dwm_install_name $dwm_install_value
    fi
    dwm_val_index=`expr $dwm_val_index + 1`
  done
  dwm_name_index=`expr $dwm_name_index + 1`
done
dwm_name_index=0
for dwm_install_name in $dwm_preset_path_names ; do
  dwm_val_index=0
  for dwm_install_value in $dwm_preset_path_values ; do
    if test "$dwm_name_index" = "$dwm_val_index"; then
      /usr/bin/printf "    %${dwm_install_max_name_len}s:  %s\n" $dwm_install_name $dwm_install_value
    fi
    dwm_val_index=`expr $dwm_val_index + 1`
  done
  dwm_name_index=`expr $dwm_name_index + 1`
done

/bin/echo ""
])

define(DWM_SET_PKGVARS,[
  AC_MSG_CHECKING([package variables])
  EPM_ARCH=""
  PKG_SUBDIR=[staging]
  TAGFULL=`./getvers.sh`
  if test "$TAGFULL" = "" ; then
    TAGFULL=$2
  fi
  TAGNAME=`echo $TAGFULL | cut -d'-' -f1`
  TAGVERSION=`echo $TAGFULL | cut -d'-' -f2-`
  TARDIR_RELATIVE=${PKG_SUBDIR}
  case $host_os in
    darwin*)
      OSNAME="darwin"
      OSVERSION=`uname -r`
      OSARCH=`uname -m`
      TAR="/usr/bin/tar"
      ;;
    freebsd*)
      OSNAME="freebsd"
      OSVERSION=`uname -r | cut -d- -f1`
      OSARCH=`uname -m`
      case $OSARCH in
	i[[3456]]86)
	  OSARCH=intel
	  ;;
	*)
	  ;;
      esac
      TAR="/usr/bin/tar"
      ;;
    linux*)
      OSNAME="linux"
      OSVERSION=`uname -r | cut -d- -f1`
      OSARCH=`uname -m`
      case $OSARCH in
	i[[3456]]86)
	  OSARCH=intel
	  ;;
	*)
	  ;;
      esac
      TAR="/bin/tar"
      unamem=`uname -m`
      case ${unamem} in
        armv7l)
          EPM_ARCH="-a armhf"
          ;;
        x86_64)
          EPM_ARCH="-a amd64"
          ;;
      esac
      ;;
    *)
      ;;
  esac
  STAGING_DIR=`pwd`/staging
  TARDIR=`pwd`/${TARDIR_RELATIVE}
  AC_SUBST(EPM_ARCH)
  AC_SUBST(OSNAME)
  AC_SUBST(OSVERSION)
  AC_SUBST(OSARCH)
  AC_SUBST(STAGING_DIR)
  AC_SUBST(TAGFULL)
  AC_SUBST(TAGNAME)
  AC_SUBST(TAGVERSION)
  AC_SUBST(PKG_SUBDIR)
  AC_SUBST(TAR)
  AC_SUBST(TARDIR)
  AC_SUBST(TARDIR_RELATIVE)
  AC_MSG_RESULT([
    OSNAME="${OSNAME}"
    OSVERSION="${OSVERSION}"
    OSARCH="${OSARCH}"
    TAGFULL="${TAGFULL}"
    TAGNAME="${TAGNAME}"
    TAGVERSION="${TAGVERSION}"
    PKG_SUBDIR="${PKG_SUBDIR}"
    TAR="${TAR}"
    TARDIR="${TARDIR}"
    TARDIR_RELATIVE="${TARDIR_RELATIVE}"])
])

dnl #------------------------------------------------------------------------
dnl #  Sets flags needed for compiling omniORB files (output from omniidl)
dnl #------------------------------------------------------------------------
define(DWM_SET_OMNIFLAGS,[
  AC_MSG_CHECKING([omniORB flags])
  case $host_os in
    freebsd[[4567]]*)
      AC_DEFINE(__freebsd__)
      OMNIFLAGS="-D__freebsd__"
      case $host_cpu in
        i?86)
          AC_DEFINE(__x86__)
          OMNIFLAGS="${OMNIFLAGS} -D__x86__"
          ;;
        *)
          echo "unknown host_cpu $host_cpu"
          ;;
        esac
      ;;
    linux*)
      AC_DEFINE(__linux__)
      OMNIFLAGS="-D__linux__"
      case $host_cpu in
        i?86)
          AC_DEFINE(__x86__)
          OMNIFLAGS="${OMNIFLAGS} -D__x86__"
          ;;
        *)
          echo "unknown host_cpu $host_cpu"
          ;;
      esac
      ;;
    solaris*)
      AC_DEFINE(__sunos__)
      AC_DEFINE(__OSVERSION__,5)
      OMNIFLAGS="-D__sunos__ -D__OSVERSION__=5"
      case $host_cpu in
        sparc)
          AC_DEFINE(__sparc__)
          OMNIFLAGS="${OMNIFLAGS} -D__sparc__"
          ;;
        *)
          echo "unknown host_cpu $host_cpu"
          ;;
      esac
      ;;
    *)
      ;;
  esac

  AC_SUBST(OMNIFLAGS)
  AC_MSG_RESULT([
    OMNIFLAGS="${OMNIFLAGS}"])
])

dnl #------------------------------------------------------------------------
dnl #  Sets flags needed for building shared library
dnl #------------------------------------------------------------------------
define(DWM_SET_CXX_SHARED_FLAGS,[
  AC_MSG_CHECKING([cxx shared flags])
  CXX_SHARED_FLAGS=""
  LD_SHARED_FLAGS=""
  SHARED_LIB_EXT=".so"
  case $host_os in
    freebsd[[9]]*)
      CXX_SHARED_FLAGS="-fPIC"
      LD_SHARED_FLAGS="-shared"
      ;;
    freebsd1[[01234]]*)
      CXX_SHARED_FLAGS="-fPIC"
      LD_SHARED_FLAGS="-shared"
      ;;
    darwin*)
      CXX_SHARED_FLAGS="-fPIC"
      LD_SHARED_FLAGS="-dynamiclib"
      SHARED_LIB_EXT=".dylib"
      ;;
    *)
      ;;
  esac
  
  AC_SUBST(CXX_SHARED_FLAGS)
  AC_SUBST(LD_SHARED_FLAGS)
  AC_SUBST(SHARED_LIB_EXT)
])


dnl #------------------------------------------------------------------------
dnl #  Sets flags needed for pthreads and threadsafe STL
dnl #------------------------------------------------------------------------
define(DWM_SET_PTHREADFLAGS,[
  AC_MSG_CHECKING([pthread flags])
  PTHREADCXXFLAGS=""
  PTHREADLDFLAGS=""
  PTHREADS_STL="-D_PTHREADS"
  case $host_os in
    freebsd[[6789]]*)
      AC_DEFINE(_THREAD_SAFE)
      AC_DEFINE(_PTHREADS)
      PTHREADS_OS="-pthread -D_THREAD_SAFE"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-pthread"
      ;;
    freebsd1[[01234]]*)
      AC_DEFINE(_THREAD_SAFE)
      AC_DEFINE(_PTHREADS)
      PTHREADS_OS="-pthread -D_THREAD_SAFE"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-pthread"
      ;;
    linux*)
      AC_DEFINE(_REENTRANT)
      AC_DEFINE(_PTHREADS)
      # AC_DEFINE(_XOPEN_SOURCE,500)
      # PTHREADS_OS="-D_REENTRANT -D_XOPEN_SOURCE=500"
      PTHREADS_OS="-pthread -D_REENTRANT"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-pthread"
      ;;
    openbsd3.[[6]]*)
      AC_DEFINE(_THREAD_SAFE)
      AC_DEFINE(_PTHREADS)
      PTHREADS_OS="-D_REENTRANT -D_THREAD_SAFE"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-pthread"
      ;;
    openbsd4*)
      AC_DEFINE(_THREAD_SAFE)
      AC_DEFINE(_PTHREADS)
      PTHREADS_OS="-pthread -D_REENTRANT -D_THREAD_SAFE"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-pthread"
      ;;
    solaris*)
      AC_DEFINE(_REENTRANT)
      AC_DEFINE(_PTHREADS)
      AC_DEFINE(_POSIX_PTHREAD_SEMANTICS)
      PTHREADS_OS="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
      PTHREADCXXFLAGS="${PTHREADS_OS} ${PTHREADS_STL}"
      PTHREADLDFLAGS="-lpthread -lposix4"
      ;;
    *)
      ;;
  esac

  AC_SUBST(PTHREADS_OS)
  AC_SUBST(PTHREADS_STL)
  AC_SUBST(PTHREADCXXFLAGS)
  AC_SUBST(PTHREADLDFLAGS)
  AC_MSG_RESULT([
    PTHREADCXXFLAGS="${PTHREADCXXFLAGS}"
    PTHREADLDFLAGS="${PTHREADLDFLAGS}"])
])

define(DWM_CHECK_STRTOF,[
  AC_CHECK_LIB(c, strtof, 
    [AC_DEFINE(HAVE_STRTOF)])
])

define(DWM_CHECK_GETHOSTBYNAME_R,[
  AC_MSG_CHECKING([for gethostbyname_r])
  OLDCPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$OLDCPPFLAGS -D_REENTRANT"
  AC_TRY_COMPILE([
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <netdb.h>],
    [
      struct hostent  *hp;
      struct hostent   result;
      char            *name = "caimis.com";
      char             buf[4096];

      hp = gethostbyname_r(name,&result,buf,4096,NULL);
    ],
    [AC_DEFINE(HAVE_GETHOSTBYNAME_R)
      AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)]
  )
  AC_SUBST(HAVE_GETHOSTBYNAME_R)
  CPPFLAGS="$OLDCPPFLAGS"
  ]
)

define(DWM_CHECK_GETHOSTBYADDR_R,[
  AC_MSG_CHECKING([for gethostbyaddr_r])
  OLDCPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$OLDCPPFLAGS -D_REENTRANT"
  AC_TRY_COMPILE([
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <netdb.h>],
    [
      struct hostent  *hp;
      struct hostent   result;
      unsigned int     ipAddr = inet_addr("127.0.0.1");
      char             buf[4096];
	
      hp = gethostbyaddr_r(&ipAddr,sizeof(ipAddr),AF_INET,&result,
			   buf,4096,NULL);
    ],
    [AC_DEFINE(HAVE_GETHOSTBYADDR_R)
      AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)]
  )
  AC_SUBST(HAVE_GETHOSTBYADDR_R)
  CPPFLAGS="$OLDCPPFLAGS"
  ]
)


dnl ######################################################################
dnl #  Sets variable named [$1] to the value of the installation prefix
dnl #  for the package named by [$1].  Substitutes for [$1] in output.
dnl #  Example: DWM_FIND_PKG_PREFIX(libCaimis) would set libCaimis
dnl #  to the installation prefix (directory) where libCaimis is 
dnl #  installed, and substitute the value for all instances of 
dnl #  @libCaimis@ in output files.
dnl ######################################################################
define(DWM_FIND_PKG_PREFIX,[
  AC_MSG_CHECKING([$1 installation prefix])
  case $host_os in
    freebsd[[4567]]*)
      [$1]=`pkg_info -p [$1\*] | grep 'CWD ' | head -1 | cut -d' ' -f3`
      ;;
    linux*)
      [$1]=`rpm -q --queryformat '%{INSTALLPREFIX}' [$1]`
      ;;
    solaris*)
      [$1]=`pkginfo -r [$1]`
      ;;
    *)
      ;;
  esac

  AC_ARG_WITH([$1],
    [  --with-$1=value   set $1 to value ([$1 prefix])],
    [$1=$withval]
  )
  AC_SUBST([$1])
  AC_MSG_RESULT($[$1])

  ]
)

dnl ------------------------------------------------------------------------
dnl //  Check for <tuple> (indicating C++0x features) or <tr1/tuple>
dnl //  (indicating C++0x features in the tr1 namespace).  These were sort
dnl //  of ugly days; we now have C++11 and I use it.
dnl ------------------------------------------------------------------------
define(DWM_CHECK_CPLUSPLUS_0x,[
  AC_MSG_CHECKING([for C++0x])
  AC_LANG_PUSH(C++)
  case ${CXX} in
    clang*)
  	prev_CPPFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -std=c++11"
	AC_MSG_CHECKING([for C++0x features])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tuple>]], [[std::tuple<int,int> t;]])],[AC_MSG_RESULT(yes)
		        AC_DEFINE(HAVE_CPP0X)],[AC_MSG_RESULT(no)
	                AC_MSG_CHECKING([for C++ TR1 features])
	        	CXXFLAGS="$prev_CPPFLAGS"
	        	_au_m4_changequote([,])AC_TRY_COMPILE([#include <tr1/tuple>],
			       	       [std::tr1::tuple<int,int> t;],
			       	       [AC_MSG_RESULT(yes)
			                AC_DEFINE(HAVE_TR1)],
			       	       [AC_MSG_RESULT(no)])])
	;;
    *)  	
	prev_CPPFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -std=gnu++0x"
	AC_MSG_CHECKING([for C++0x features])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tuple>]], [[std::tuple<int,int> t;]])],[AC_MSG_RESULT(yes)
	                AC_DEFINE(HAVE_CPP0X)],[AC_MSG_RESULT(no)
	                AC_MSG_CHECKING([for C++ TR1 features])
	        	CXXFLAGS="$prev_CPPFLAGS"
	        	_au_m4_changequote([,])AC_TRY_COMPILE([#include <tr1/tuple>],
			               [std::tr1::tuple<int,int> t;],
			       	       [AC_MSG_RESULT(yes)
			                AC_DEFINE(HAVE_TR1)],
			       	       [AC_MSG_RESULT(no)])])
    ;;
  esac
AC_LANG_POP()
])

dnl ########################################################################
dnl #  Macro to check for the existence of C++11.  Note that this depends
dnl #  on the existence of a '-std=c++11' command line option of the C++
dnl #  compiler.  Should work for both g++ and clang++.
dnl ########################################################################
define(DWM_CHECK_CPLUSPLUS_11,[
  AC_MSG_CHECKING([for C++11])
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++11"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <iostream>
    #include <vector>]], [[std::vector<int> vi = { 1, 2, 3, 4 };
     for (auto i : vi) { std::cout << i << '\n'; }]])],[AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_CPP11)],[AC_MSG_RESULT(no)
     echo C++11 is required\!\!
     exit 1])
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_CPLUSPLUS_1Z,[
  AC_MSG_CHECKING([for C++1z])
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++1z"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <shared_mutex>
    #include <vector>]], [[std::shared_mutex  mtx;
     std::shared_lock<std::shared_mutex>  lock(mtx);]])],[AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_CPP1Z)],[AC_MSG_RESULT(no)
     CXXFLAGS="$prev_CPPFLAGS"
     DWM_CHECK_CPLUSPLUS_11])
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_CPLUSPLUS_17,[
  AC_MSG_CHECKING([for C++17])
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++17"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <shared_mutex>
    #include <vector>]], [[std::shared_mutex  mtx;
     std::shared_lock<std::shared_mutex>  lock(mtx);]])],[AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_CPP17)
     AC_DEFINE(HAVE_CPP11)
     LDFLAGS="$LDFLAGS -std=c++17"],[AC_MSG_RESULT(no)
     CXXFLAGS="$prev_CPPFLAGS"
     DWM_CHECK_CPLUSPLUS_1Z()])
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_CPLUSPLUS_20,[
  AC_MSG_CHECKING([for C++20])
  AC_LANG_PUSH(C++)
  AX_CHECK_COMPILE_FLAG([-std=c++20], [
    AC_DEFINE(HAVE_CPP20)
    AC_DEFINE(HAVE_CPP17)
    AC_DEFINE(HAVE_CPP11)
    CXXFLAGS="$CXXFLAGS -std=c++20"
    LDFLAGS="$LDFLAGS -std=c++20"
  ], [
    DWM_CHECK_CPLUSPLUS_17()
  ])
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_CPLUSPLUS_23,[
  AC_MSG_CHECKING([for C++23])
  AC_LANG_PUSH(C++)
  AX_CHECK_COMPILE_FLAG([-std=c++23], [
    AC_DEFINE(HAVE_CPP23)
    AC_DEFINE(HAVE_CPP20)
    AC_DEFINE(HAVE_CPP17)
    AC_DEFINE(HAVE_CPP11)
    CXXFLAGS="$CXXFLAGS -std=c++23"
    LDFLAGS="$LDFLAGS -std=c++23"
  ], [
    DWM_CHECK_CPLUSPLUS_20()
  ])
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_SQLITE3,[
  AC_MSG_CHECKING([for sqlite3])
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -I/usr/src/contrib/sqlite3"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <sqlite3.h>]], [[sqlite3  *ppdb;]])],[AC_MSG_RESULT(yes)
     AC_DEFINE(HAVE_SQLITE3)
     SQLITEDIR=/usr/src/contrib/sqlite3],[AC_MSG_RESULT(no)
     echo '/usr/src/contrib/sqlite3 is required (FreeBSD src)'
     exit 1])
  CXXFLAGS="$prev_CPPFLAGS"
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_NEED_LIBATOMIC,[
  AC_MSG_CHECKING([if we need libatomic])
  AC_LANG_PUSH(C++)
  prev_OSLIBS="$OSLIBS"
  prev_LDFLAGS="$LDFLAGS"
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE([[
     #include <atomic>
     int main() {
       std::atomic<bool>  foo;
         std::atomic<uint64_t>  count(0);
         while (foo.exchange(true)) {
	   count += 1;
	 }
       bool  bar = false;
       foo = bar;
     }
    ]])],
    [AC_MSG_RESULT(no)],
    [
     OSLIBS="$OSLIBS -latomic"
     LDFLAGS="$LDFLAGS $OSLIBS"
     AC_LINK_IFELSE(
       [AC_LANG_SOURCE([[
        #include <atomic>
        int main() {
          std::atomic<bool>  foo;
          std::atomic<uint64_t>  count(0);
          while (foo.exchange(true)) {
 	    count += 1;
 	  }
          bool  bar = false;
	  foo = bar;
        }
       ]])],
       [AC_MSG_RESULT(yes)],
       [AC_MSG_RESULT(no)
        OSLIBS="$prev_OSLIBS"]
     )
    ]
  )
  LDFLAGS="$prev_LDFLAGS"
  AC_LANG_POP()
])
    
dnl #-------------------------------------------------------------------------
define(DWM_CHECK_NEED_LIBIBVERBS,[
  AC_MSG_CHECKING([if we need libibverbs])
  AC_LANG_PUSH(C++)
  NEED_LIBIBVERBS=0
  prev_LDFLAGS="$LDFLAGS"
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE([[
     #include <infiniband/verbs.h>
     int main() {
       struct ibv_device   ibvDev;
       struct ibv_context *ibvCtx = ibv_open_device(&ibvDev);
     }
    ]])],
    [AC_MSG_RESULT(no)],
    [NEED_LIBIBVERBS=1]
  )
  if [[ $NEED_LIBIBVERBS == 1 ]] ; then
    LDFLAGS="$LDFLAGS -libverbs"
    AC_LINK_IFELSE(
      [AC_LANG_SOURCE([[
       #include <infiniband/verbs.h>
       int main() {
         struct ibv_device   ibvDev;
         struct ibv_context *ibvCtx = ibv_open_device(&ibvDev);
       }
      ]])],
      [AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)
       LDFLAGS="$prev_LDFLAGS"]
    )
  fi
  AC_LANG_POP()
])

dnl #-------------------------------------------------------------------------
define(DWM_CHECK_NEED_LIBRT,[
  AC_MSG_CHECKING([if we need librt])
  AC_LANG_PUSH(C++)
  NEED_LIBRT=0
  prev_LDFLAGS="$LDFLAGS"
  prev_OSLIBS="$OSLIBS"
  LDFLAGS="$LDFLAGS $OSLIBS"
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE([[
     extern "C" {
     #include <aio.h>
     }
     #include <iostream>
     int main() {
       struct aiocb  myIocb;
       if (aio_write(&myIocb) == 0) {
         std::cerr << "success\n";
       }
     }
    ]])],
    [AC_MSG_RESULT(no)
     LDFLAGS="$prev_LDFLAGS"
     OSLIBS="$prev_OSLIBS"],
    [
     OSLIBS="$OSLIBS -lrt"
     LDFLAGS="$prev_LDFLAGS $OSLIBS"
     AC_LINK_IFELSE(
       [AC_LANG_SOURCE([[
        extern "C" {
	#include <aio.h>
	}
	#include <iostream>
        int main() {
	  struct aiocb  myIocb;
	  if (aio_write(&myIocb) == 0) {
            std::cerr << "success\n";
	  }
        }
       ]])],
       [AC_MSG_RESULT(yes)],
       [AC_MSG_RESULT(no)
        LDFLAGS="$prev_LDFLAGS"
	OSLIBS="$prev_OSLIBS"]
     )
    ]
  )
  LDFLAGS="$prev_LDFLAGS"
  AC_LANG_POP()
])

define(DWM_CHECK_PCAP,[

])

dnl #------------------------------------------------------------------------
define(DWM_CHECK_LIBSTDCPPFS,[
  AC_MSG_CHECKING([for libstdc++fs])
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  prev_LIBS="${LIBS}"
  CXXFLAGS="$CXXFLAGS -std=c++17"
  LIBS="${LIBS} -lstdc++fs"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <filesystem>]], [[std::filesystem::remove("foo");]])],[AC_MSG_RESULT(yes)
               AC_SUBST(LIBSTDCPPFS,[-lstdc++fs])],[AC_MSG_RESULT(no)
               LIBS="$prev_LIBS"])
  CXXFLAGS="$prev_CPPFLAGS"
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(DWM_CHECK_LIBDWM,[
  AC_MSG_CHECKING([for libDwm])
  AC_LANG_PUSH(C++)
  prev_CXXFLAGS="$CXXFLAGS"
  libDwmCxxFlags=`pkg-config --cflags libDwm 2>/dev/null`
  CXXFLAGS="${CXXFLAGS} ${libDwmCxxFlags}"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include "DwmSysLogger.hh"]],
    [[Dwm::SysLogger::ShowFileLocation(true);]])],
    [AC_MSG_RESULT(found)],
    [AC_MSG_RESULT(not found)
      echo libDwm is required \!\!
      exit 1])
  CXXFLAGS="${prev_CXXFLAGS}"
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(DWM_CHECK_LIBDWMCREDENCE,[
  AC_MSG_CHECKING([for libDwmCredence])
  AC_LANG_PUSH(C++)
  prev_CXXFLAGS="$CXXFLAGS"
  libDwmCredenceCxxFlags=`pkg-config --cflags libDwmCredence 2>/dev/null`
  CXXFLAGS="${CXXFLAGS} ${libDwmCredenceCxxFlags}"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include "DwmCredencePeer.hh"]],
    [[Dwm::Credence::Peer peer;]])],
    [AC_MSG_RESULT(found)],
    [AC_MSG_RESULT(not found)
      echo libDwmCredence is required \!\!
      exit 1])
  CXXFLAGS="${prev_CXXFLAGS}"
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(DWM_CHECK_LIBDWMWEBUTILS,[
  AC_MSG_CHECKING([for libDwmWebUtils])
  AC_LANG_PUSH(C++)
  prev_CXXFLAGS="$CXXFLAGS"
  libDwmWebUtilsCxxFlags=`pkg-config --cflags libDwmWebUtils 2>/dev/null`
  CXXFLAGS="${CXXFLAGS} ${libDwmWebUtilsCxxFlags}"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include "DwmWebUtilsUrl.hh"]],
    [[Dwm::WebUtils::Url  url;]])],
    [AC_MSG_RESULT(found)],
    [AC_MSG_RESULT(not found)
      echo libDwmWebUtils is required \!\!
      exit 1])
  CXXFLAGS="${prev_CXXFLAGS}"
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(DWM_COMPILE_BOOSTASIO,[
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  if [[ -n "$1" ]]; then
    CXXFLAGS="$CXXFLAGS -I[$1]/include"
  fi
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[#include <boost/asio.hpp>]],
      [[boost::asio::ip::tcp::iostream  tcpStream;]])
    ],
    [BOOSTDIR="$1"],
    [BOOSTDIR="none"
     CXXFLAGS="$prev_CPPFLAGS"]
  )
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(DWM_CHECK_BOOSTASIO,[
  AC_MSG_CHECKING([for Boost asio])
  for boost_dir in "" "/usr/local" "/opt/local" "/opt/local/libexec/boost/1.88"; do
    DWM_COMPILE_BOOSTASIO([${boost_dir}])
    if [[ "${BOOSTDIR}" != "none" ]]; then
      break
    fi
  done
  if [[ "${BOOSTDIR}" != "none" ]]; then
    AC_MSG_RESULT([found ${BOOSTDIR}])
    AC_DEFINE(HAVE_BOOSTASIO)
    if [[ -n "${BOOSTDIR}" ]]; then
      BOOSTINC=-I${BOOSTDIR}/include
      BOOSTLIBS="-L${BOOSTDIR}/lib"
    fi
    if [[ -f ${BOOSTDIR}/lib/libboost_system-mt.dylib ]]; then
      BOOSTLIBS="${BOOSTLIBS} -lboost_iostreams-mt -lboost_system-mt"
      BOOSTLIBTAG="-mt"
    else
      BOOSTLIBS="${BOOSTLIBS} -lboost_iostreams -lboost_system"
    fi
    AC_SUBST(BOOSTDIR)
    AC_SUBST(BOOSTINC)
    AC_SUBST(BOOSTLIBS)
    AC_SUBST(BOOSTLIBTAG)
  else
    echo Boost asio is required\!\!
    exit 1
  fi
])

dnl #-------------------------------------------------------------------------
define(DWM_GIT_TAG,[
  gittag=`git describe --tags --dirty 2>/dev/null`
  dirty=`echo "${gittag}" | awk -F '-' '{ if (NF > 2) { print "dirty"; } }'`
  if test -z "${dirty}"; then
    GIT_TAG="${gittag}"
    GIT_VERSION=`echo "${gittag}" | awk -F '-' '{print $NF}'`
  else
    fakevers=`date +%Y%m%d`
    GIT_TAG="[$1]-0.0.${fakevers}"
    GIT_VERSION="0.0.${fakevers}"
  fi
  AC_SUBST(GIT_TAG)
  AC_SUBST(GIT_VERSION)
])

dnl #-------------------------------------------------------------------------
define(DWM_SVN_TAG,[
  svntag=`svn info . 2>/dev/null | grep ^URL | grep 'tags/' | awk -F 'tags/' '{print [$]2}' | awk -F '/' '{print [$]1}'`
  if test -n "${svntag}"; then
    SVN_TAG="${svntag}"
    SVN_VERSION=`echo [${svntag}] | awk -F '-' '{print $NF}'`
  else
    svn_vers=`svnversion . | sed 's/[[0-9]]*://g'`
    if test "${svn_vers}" = "Unversioned directory"; then
      SVN_VERSION="0.0.0"
    else
      SVN_VERSION="0.0.${svn_vers}"
    fi
    SVN_TAG="[$1]-${SVN_VERSION}"
  fi
  AC_SUBST(SVN_TAG)
  AC_SUBST(SVN_VERSION)
])

dnl #-------------------------------------------------------------------------
define(DWM_GET_TAG,[
  DWM_GIT_TAG([$1])
  if test -n "${GIT_TAG}" ; then
    DWM_TAG="${GIT_TAG}"
    DWM_VERSION="${GIT_VERSION}"
  else
    DWM_SVN_TAG([$1])
    if test -n "${SVN_TAG}" ; then
      DWM_TAG="${SVN_TAG}"
      DWM_VERSION="${SVN_VERSION}"
    fi
  fi
  if test -z "${DWM_TAG}" ; then
    DWM_TAG="[$1]-0.0.0"
    DWM_VERSION="0.0.0"
  fi
  DWM_NAME=`echo "${DWM_TAG}" | sed s/\-${DWM_VERSION}//g`
  AC_SUBST(DWM_TAG)
  AC_SUBST(DWM_VERSION)
  AC_SUBST(DWM_NAME)
])

dnl --------------------------------------------------------------------------
define(DWM_ADD_IF_NOT_PRESENT,[
  inc_found=0
  for inc in ${$1} ; do
    if [[ "$2" = "${inc}" ]]; then
      inc_found=1
      break
    fi
  done
  if [[ ${inc_found} -eq 0 ]]; then
    if [[ -n "${$1}" ]]; then
      [$1]="${$1}[$3][$2]"
    else
      [$1]="[$2]"
    fi
  fi
])

dnl --------------------------------------------------------------------------
define(DWM_CHECK_PKG,[
  AC_MSG_CHECKING([for [$1] pkg])
  DWM_HAVE_[$1]_PKG=0
  pkg-config --exists [$1]
  if [[ $? -eq 0 ]]; then
    DWM_HAVE_[$1]_PKG=1
    DWM_ADD_IF_NOT_PRESENT(PC_PKGS,[$1],[ ])
    DWM_ADD_IF_NOT_PRESENT(PC_REQ_PKGS,[$1],[, ])
    AC_MSG_RESULT([found])
  else
    AC_MSG_RESULT([not found])
    [$2]
  fi
])

dnl --------------------------------------------------------------------------
define(DWM_CHECK_STD_FORMAT,[
  AC_MSG_CHECKING([for working std::format])
  AC_LANG_PUSH(C++)
  AC_LINK_IFELSE(
    [AC_LANG_SOURCE([[
      #include <format>
      template <typename ...Args>
      static std::string FmtTest(std::format_string<Args...> fmt,
                                 Args&&... args)
      { return std::format(fmt,std::forward<Args>(args)...); }
      int main(int argc, char *argv[])
      {
        if (FmtTest("{} {}", 1, "hey") == "1 hey") { return 0; }
        else { return 1; }
      }
    ]])],
    [AC_MSG_RESULT([yes])
     DWM_HAVE_STD_FORMAT=1
     AC_DEFINE(DWM_HAVE_STD_FORMAT)],
    [AC_MSG_RESULT([no])
     DWM_HAVE_STD_FORMAT=0]
  )
  AC_LANG_POP()
])

dnl --------------------------------------------------------------------------
define(DWM_CHECK_LIBFMT,[
  if [[ ${DWM_HAVE_fmt_PKG} -eq 1 ]]; then
    AC_MSG_CHECKING([for working fmt::format])
    AC_LANG_PUSH(C++)
    prev_LIBS="${LIBS}"
    LIBS=`pkg-config --libs fmt`
    prev_CXXFLAGS="${CXXFLAGS}"
    CXXFLAGS=`pkg-config --cflags fmt`
    AC_LINK_IFELSE(
      [AC_LANG_SOURCE([[
        #include <fmt/format.h>
        template <typename ...Args>
        static std::string FmtTest(fmt::format_string<Args...> f,
                                   Args&&... args)
        { return fmt::format(f,std::forward<Args>(args)...); }
        int main(int argc, char *argv[])
        {
          if (FmtTest("{} {}", 1, "hey") == "1 hey") { return 0; }
          else { return 1; }
        }
      ]])],
      [AC_MSG_RESULT([yes])
       DWM_HAVE_LIBFMT=1
       AC_DEFINE(DWM_HAVE_LIBFMT)],
      [AC_MSG_RESULT([no])
       DWM_HAVE_LIBFMT=0]
    )
    LIBS="${prev_LIBS}"
    CXXFLAGS="${prev_CXXFLAGS}"
    AC_LANG_POP()
  else
       DWM_HAVE_LIBFMT=0
  fi
])
