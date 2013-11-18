#!/bin/sh
mkdir -p po m4

echo "gettextize..."
echo "no" | gettextize --force --copy --no-changelog

echo "intltoolize..."
intltoolize --copy --force --automake

echo "libtoolize"
libtoolize
echo "aclocal..."
aclocal -I m4

echo "autoconf..."

autoconf

echo "autoheader..."
autoheader

echo "automake..."
automake --add-missing --copy --gnu



