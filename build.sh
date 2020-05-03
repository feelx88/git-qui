#!/bin/bash

if [[ -z $GIT_QUI_OS ]]; then
  echo "Please set the GIT_QUI_OS environment variable to specify the build output folder (linux/mac)."
  exit 1
fi

if [[ -z $VERSION ]]; then
  echo "Please set the VERSION environment variable to specify the build version."
  exit 1
fi

set -e

mkdir -p dist/build
mkdir -p dist/installer
cd dist/build

qmake ../../git-qui.pro

make
make clean

if [[ $GIT_QUI_OS = "linux" ]]; then
  linuxdeployqt git-qui
  echo "###"
elif [[ $GIT_QUI_OS = "mac" ]]; then
  macdeployqt git-qui.app
fi

archivegen git-qui .

cd ..
cp -r ../deploy/installer/* installer

mkdir installer/packages/git-qui/data
mv build/git-qui.7z installer/packages/git-qui/data

repogen -p installer/packages $GIT_QUI_OS
binarycreator -c installer/config-$GIT_QUI_OS.xml -p installer/packages installer-$GIT_QUI_OS-$(uname -m)

if [[ $GIT_QUI_OS = "linux" ]]; then
  mv installer-$GIT_QUI_OS-$(uname -m) $GIT_QUI_OS
elif [[ $GIT_QUI_OS = "mac" ]]; then
  macdeployqt installer-$GIT_QUI_OS-$(uname -m).app -dmg
  mv installer-$GIT_QUI_OS-$(uname -m).dmg $GIT_QUI_OS
  rm -rf installer-$GIT_QUI_OS-$(uname -m).app
fi

zip -r $GIT_QUI_OS.zip $GIT_QUI_OS
rm -rf $GIT_QUI_OS
