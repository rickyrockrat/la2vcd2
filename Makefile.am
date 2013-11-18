#PACKAGE_VERSION = @PACKAGE_VERSION@

#AM_CFLAGS( -Wall
# set the options for automake..
#Hook dist-bzip2 to dist (dist-bzip2).
#set gnu strictness (gnu)
AUTOMAKE_OPTIONS =  dist-bzip2 

bin_PROGRAMS = la2vcd2
la2vcd2_SOURCES = la2vcd.c
la2vcd2_LDADD = libla2vcd2.la

include_HEADERS = la2vcd2.h
lib_LTLIBRARIES = libla2vcd2.la
libla2vcd2_la_SOURCES =  la2vcd_lib.c
libla2vcd2_la_LDFLAGS=-version-info 0:0:0


# pkg-config
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libla2vcd2.pc

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh po/Makefile.in.in po/POTFILES.in
dist_doc_DATA = README
DISTCLEANFILES = po/Makefile po/Makefile.in po/stamp-it po/POTFILES


