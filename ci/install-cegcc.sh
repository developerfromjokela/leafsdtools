#!/bin/sh
# Installs the cegcc mingw32ce 0.59.1 cross toolchain (GCC 4.4, 32-bit x86
# binaries) into the directory given as the first argument. Meant for
# Debian/Ubuntu on x86-64; needs root for apt.
#
# The compiler links against libgmp.so.3 and libmpfr.so.1, which current
# distributions no longer ship, so they are taken from archive.debian.org and
# placed next to the toolchain. Use the printed PATH and LD_LIBRARY_PATH.
set -eu

PREFIX=${1:?usage: install-cegcc.sh PREFIX}
SF=https://downloads.sourceforge.net/project/cegcc/cegcc/0.59.1
DEB=http://archive.debian.org/debian/pool/main

fetch() { # url sha256 file
    wget -q -O "$3" "$1"
    echo "$2  $3" | sha256sum -c -
}

dpkg --add-architecture i386
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
    wget ca-certificates bzip2 make libc6:i386 zlib1g:i386

mkdir -p "$PREFIX/lib32"
tmp=$(mktemp -d)
cd "$tmp"

fetch "$SF/mingw32ce-0.59.1.tar.bz2" \
    dd7e08204330323fb392b83aeb16abdf9d165eeebd9bcb93dc7f9560b8027470 mingw32ce.tar.bz2
fetch "$DEB/g/gmp/libgmp3c2_4.2.1+dfsg-4_i386.deb" \
    c543215f2fa06b2e3be29c743fa60179cdae186e9d7bc78a6598bea974221da5 gmp.deb
fetch "$DEB/m/mpfr/libmpfr1ldbl_2.3.1.dfsg.1-2_i386.deb" \
    182704d3cfa9d8c719094525ca8f7a6d4417d83a3a8099d33b905be02100ea3d mpfr.deb

tar xjf mingw32ce.tar.bz2 -C "$PREFIX" --strip-components=2
dpkg-deb -x gmp.deb x
dpkg-deb -x mpfr.deb x
cp -P x/usr/lib/libgmp.so.3* x/usr/lib/libmpfr.so.1* "$PREFIX/lib32/"

cd /
rm -rf "$tmp"
echo "PATH=$PREFIX/bin"
echo "LD_LIBRARY_PATH=$PREFIX/lib32"
