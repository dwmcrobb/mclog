# mclog

mclog is a logging system targetted at C++ applications.  It provides
functionality similar to to `syslogd(8)` and `syslog(3)`, but provides
some additional flexibility on the client side (configurable message
sinks), encrypted multicast network transport, and built-in log file
rollover.  It also has more log message routing granularity via a
fairly simple message filter grammar.

Unlike `syslog(3)`, the main API utilizes `std::format` to avoid the
pitfalls of a printf-style format string and C varargs.

## Components

### mclogd
`mclogd` is a daemon that can accept log messages on the
loopback interface of the host, accept encrypted log messages via
multicast, send encrypted log messages via multicast, and of course
save log messages to files.  For most applications using mclog,
`mclogd` is the log message destination (via the loopback
interface).

See the `mclogd(8)` and `mclogd.cfg(5)` manpages for more information.

### C++ library (libDwmMclog)
`libDwmMclog` provides the classes and macros that may be used to send
log messages to `mclogd(8)` or directly to files.  It may also be used
to log messages to any object that provides a `Dwm::Mclog::MessageSink`
interface.

### mclog
`mclog` is a simple command line application that may be used to view
log messages from multicast in real time as well as log messages saved
to files in binary form.

## Prerequisites for building
- GNU make
- flex
- bison
- [dwm_gmk](https://github.com/dwmcrobb/dwm_gmk) GNU make plugin
- [mkfbsdmnfst](https://github.com/dwmcrobb/mkfbsdmnfst) for creating a FreeBSD package
- [mkdebcontrol](https://github.com/dwmcrobb/mkdebcontrol) for ceating a Debian package (`.deb`)
- [dwmwhat](https://github.com/dwmcrobb/dwmwhat)
- [libCredence](https://github.com/dwmcrobb/libCredence)
- [libDwm](https://github.com/dwmcrobb/libDwm)
- libsodium
- boost libraries (boost_regex, boost_iostreams, boost_system)
- xxhash
- libpcap
- libz
- bzip2 library
