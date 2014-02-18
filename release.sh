#!/bin/bash
# \file ******************************************************************
#\n\b File:        release.sh
#\n\b Author:      Doug Springer
#\n\b Company:     DNK Designs Inc.
#\n\b Date:        02/17/2013 
#\n\b Description: File to tag, prep, tar, and copy to sourceforge
#*/ /************************************************************************
# This file is part of la2vcd2.
# For details, see http://la2vcd2.sourceforge.net/projects
#
# Copyright (C) 2008-2014 Doug Springer <gpib@rickyrockrat.net>
#
#    OpenGPIB is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License version 3 
#    as published by the Free Software Foundation. Note that permission 
#    is not granted to redistribute this program under the terms of any
#    other version of the General Public License.
#
#    OpenGPIB is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with OpenGPIB.  If not, see <http://www.gnu.org/licenses/>.
#    
#		The License should be in the file called COPYING.
#
PURL="https://svn.code.sf.net"
PROJ=la2vcd2

BURL="$PURL/p/$PROJ/code"
TO_DIR="/home/frs/project/$PROJ/$PROJ"
CDIR=$(pwd)
ASVN=$(dirname $CDIR)
#ASVN=$$(echo "$CDIR"|sed 's!.*/!!')
log () {
echo "$1" 
}
#****************************
# description
#****************************
check_err () {
  if [ $? -ne 0 ]; then
	log "$1 Failed. Abort"
	exit 1
  fi
}
#****************************
# Make sure we are checked in.
#****************************
check_local () {
  echo "$FUNCNAME"
  M=$(svn st|grep -c "^M")
  if [ "$M" != "0" ]; then
    echo "local is not up to date"
    if [ "$1" = "1" ]; then
      return
    fi
    exit 1
  fi
}

#****************************
#
#****************************
ask_version () {
  echo "$FUNCNAME"
  if [ -n "$tag" ]; then
    return
  fi
  if [ -z "$1" ]; then
    echo "Tag to operate on (copy to or build deb from)?"
    read tag
  else
    tag="$1"
  fi
  turl="$BURL/tags/$tag"
  tdir="$PROJ-$tag"
  CODIR="$ASVN/test.$tag"
#  mkdir -p $CODIR
  LOGF="$CODIR/build.log"
  debdir="$PROJ_$tag"
  TARF=$CODIR/$PROJ-${tag}.tar.gz
  tlist="$CODIR/README.txt  $TARF"
  NDIR="$TO_DIR/$PROJ-$tag"
}
#****************************
# Ask username for sourceforge
#****************************
ask_user () {
  log "$FUNCNAME"
  if [ -n "$user" ]; then
    return
  fi
  log "username?"
  read user
}

#****************************
# Copy the trunk to tag
#****************************
svn_cp_to_tag () {
  echo "$FUNCNAME"
  ask_version
  E=$(svn ls $turl --depth empty 2>&1|grep -c "non-exist")
  if [ "$E" = "0" ]; then
    echo "$turl exists."
    exit 1
  fi
  if [ -e "$CODIR" ]; then
    echo "$CODIR exists. Refusing to overwrite."
    exit 1
  fi
  echo "Description for tag?"
  read desc
  ask_user
  echo "Contacting $PURL for svn cp"
  svn cp --username "$user" -m"$desc" $BURL/trunk $turl
  check_err "svn copy"
  mkdir -p $CODIR
}
#****************************
# make check out dir and and check out tag
#****************************
test_tag_co () {
  echo "$FUNCNAME"
  mkdir -p "$CODIR"
  check_err "mkdir co"
  cd "$CODIR"
  if [ -e $CODIR/$tag/.svn ] ; then
    log "$CODIR/$tag already exists. Not re-checking out"
    return
  fi
  log "performing svn co $turl"
  svn co "$turl" $tdir
  check_err "svn co"
  cd $tdir
  check_err "cd $tdir"
  update_configure_ac
  ./autogen.sh
  rm *~
}
#****************************
# Make the tarball
#****************************
make_tarball () {
  log "$FUNCNAME"
  cd "$CODIR"
  check_err "cd $CODIR"
  tar --exclude .svn -czf $tdir.tar.gz $tdir
  check_err "tar -c"
}
#****************************
# test the tar ball
#****************************
test_tarball() {
  log "$FUNCNAME"
  cd "$CODIR"
  check_err "cd dir"
  mkdir tar_test
  check_err "mkdir tar_test"
  cd tar_test
  tar -xzf ../$tdir.tar.gz
  check_err "untar"
  cd "$tdir"
  check_err "cd tardir"
  ./configure
  check_err "config"
  make
  check_err "make"
}
#****************************
#
#****************************
copy_files_to_sourceforge () {
 ask_version 
 echo -n "Username for sourceforge: "
 ask_user
# Send the command to create a shell
 echo "When you get to command line, type exit"
 ssh -t $user,$PROJ@shell.sourceforge.net create
 echo "Creating directory $NDIR"
 ssh $user@shell.sourceforge.net mkdir -p $NDIR
 cp $CDIR/README $CODIR/README.txt
 echo "" >> $CODIR/README.txt
 echo "" >> $CODIR/README.txt
 cat $CDIR/ChangeLog >> $CODIR/README.txt
 tlist="$CODIR/README.txt  $TARF"
 echo "Copy files $tlist"
 echo "To $NDIR"
 scp $tlist $user@shell.sourceforge.net:$NDIR
}
#****************************
#****************************
update_configure_ac () {
AC_INIT=$(grep AC_INIT configure.ac)
NEW_AC_INIT=$(echo "$AC_INIT" |sed "s#\(.*\[.*\[\)\(.*\)\(\].*\[.*\)#\1$tag\3#")
if [ "$AC_INIT" != "$NEW_AC_INIT" ]; then
        echo "Modifying configure.ac from:"
        echo "$AC_INIT"
        echo "To"
        echo "$NEW_AC_INIT"
        echo "Continue?"
        read YN
        if [ "$YN" != "y" ]; then
                echo "User abort"
                exit 1
        fi
        sed -i "s#AC_INIT.*#$NEW_AC_INIT#" configure.ac
#        echo "msg for modification of ac_config?"
#        read msg
#        echo "may need passwd for svn:"
#        svn ci -m"$msg" configure.ac
else
	echo "configure.ac is at $tag"
fi

}
check_local
ask_version 
echo "CODIR=$CODIR"
svn_cp_to_tag
test_tag_co
make_tarball
test_tarball
copy_files_to_sourceforge
