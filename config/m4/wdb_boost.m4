AC_DEFUN([WDB_BOOST_CHECK],
[
req_boost_version=ifelse([$1], [], [1.33.1], [$1])
AX_BOOST_BASE($req_boost_version)
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM
AX_BOOST_THREAD

# Settings
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
export CPPFLAGS
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
export LDFLAGS
LIBS="$LIBS $BOOST_PROGRAM_OPTIONS_LIB $BOOST_DATE_TIME_LIB $BOOST_REGEX_LIB $BOOST_FILESYSTEM_LIB $BOOST_THREAD_LIB -lboost_system"
export LIBS
])

