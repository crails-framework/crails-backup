#!/usr/bin/env -S bash -e

PACKAGE_NAME="crails-backup"
CRAILS_VERSION=master
CPPGET_FINGERPRINT="70:64:FE:E4:E0:F3:60:F1:B4:51:E1:FA:12:5C:E0:B3:DB:DF:96:33:39:B9:2E:E5:C2:68:63:4C:A6:47:39:43"
BUILD_DIR="/tmp/build-$PACKAGE_NAME"
DEFAULT_INSTALL_ROOT=/usr/local

echo "             _ _         _           _"
echo " ___ ___ ___|_| |___ ___| |_ ___ ___| |_ _ _ ___"
echo "|  _|  _| .'| | |_ -|___| . | .'|  _| '_| | | . |"
echo "|___|_| |__,|_|_|___|   |___|__,|___|_,_|___|  _|"
echo "                                            |_|"

if [ -z "$COMPILER" ] ; then
  if   which clang++ > /dev/null 2>&1 ; then export DEFAULT_COMPILER=clang++
  elif which g++     > /dev/null 2>&1 ; then export DEFAULT_COMPILER=g++
  fi
  COMPILER=$DEFAULT_COMPILER
fi

if [ -z "$INSTALL_ROOT" ] ; then
  INSTALL_ROOT=$DEFAULT_INSTALL_ROOT
fi

touch "$INSTALL_ROOT/toto" && rm "$INSTALL_ROOT/toto" && USE_SUDO=n || USE_SUDO=y
if [ "$USE_SUDO" = "y" ] ; then
  SUDO_OPTION="config.install.sudo=sudo"
fi

system_packages=(
)

if [ -f "/usr/include/boost/any.hpp" ] ; then
  echo "+ Looks like boost is already installed"
  system_packages+=(?sys:libboost-process)
  system_packages+=(?sys:libboost-program-options)
  system_packages+=(?sys:libboost-date-time)
fi

##
## Prepare build2
##
if ! which bpkg ; then
  echo "+ bpkg does not appear to be installed. Installing build2:"
  BUILD2_VERSION="0.16.0"
  curl -sSfO https://download.build2.org/$BUILD2_VERSION/build2-install-$BUILD2_VERSION.sh
  chmod +x build2-install-$BUILD2_VERSION.sh
  sh build2-install-$BUILD2_VERSION.sh
fi

##
## Build crails-backup
##
echo "+ creating package at $BUILD_DIR"

if [ -d "$BUILD_DIR" ] ; then
  echo "++ removing previously existing package at $BUILD_DIR"
  rm -rf "$BUILD_DIR"
fi

bpkg create -d "$BUILD_DIR" cc \
  config.cxx=$COMPILER \
  config.cc.coptions=-O3 \
  config.bin.rpath="$INSTALL_ROOT/lib" \
  config.install.root="$INSTALL_ROOT"  \
  $SUDO_OPTION

cd "$BUILD_DIR"
bpkg add "https://github.com/crails-framework/$PACKAGE_NAME.git#$CRAILS_VERSION"
bpkg fetch --trust "$CPPGET_FINGERPRINT"
bpkg build $PACKAGE_NAME --yes ${system_packages[@]}
bpkg install $PACKAGE_NAME
