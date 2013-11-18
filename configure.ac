# -------------------------------------------------------------------------------
#
# -------------------------------------------------------------------------------
AC_PREREQ([2.5])
AC_INIT([la2vcd2], [0.4], [gpib@rickyrockrat.net])
#AM_INIT_AUTOMAKE([-Wall -Werror foreign], [AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
AM_INIT_AUTOMAKE([AC_PACKAGE_NAME()], [AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([la2vcd.c])
AM_CONFIG_HEADER([config.h])
AC_PROG_INTLTOOL([0.23])
# Check for header files
AC_CHECK_HEADERS([stdlib.h string.h ctype.h errno.h stdarg.h stdbool.h stdint.h stdio.h time.h])

# Add function to ACxx to prevent AC_DEFINE followed by AC_SUBST
# AC_DEFINE_SUBST(NAME, VALUE, DESCRIPTION)
# -----------------------------------------
AC_DEFUN([AC_DEFINE_SUBST], [AC_DEFINE([$1], [$2], [$3])AC_SUBST([$1], ['$2'])])

# Library versioning
lt_current="0"
lt_revision="0"
lt_age="0"
AC_SUBST(lt_current)
AC_SUBST(lt_revision)
AC_SUBST(lt_age)

# our version
#AC_DEFINE_SUBST([VERSION], [AC_PACKAGE_VERSION], [Program Version])
VERSION=AC_PACKAGE_VERSION

#Check for tools:
AC_PROG_CC
#AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

#Add support for --enable/--disable-shared
AM_PROG_LIBTOOL

AC_CONFIG_FILES([Makefile libla2vcd2.pc])

AC_OUTPUT(po/Makefile)


