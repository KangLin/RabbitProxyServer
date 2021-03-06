#!/bin/bash
set -e

SOURCE_DIR=`pwd`

if [ -n "$1" ]; then
    VERSION=`git describe --tags`
    if [ -z "$VERSION" ]; then
        VERSION=` git rev-parse HEAD`
    fi
    
    echo "Current verion: $VERSION, The version to will be set: $1"
    echo "Please check the follow list:"
    echo "    - Test is ok ?"
    echo "    - Translation is ok ?"
    echo "    - Setup file is ok ?"
    echo "    - Update_*.xml is ok ?"
    
    read -t 30 -p "Be sure to input Y, not input N: " INPUT
    if [ "$INPUT" != "Y" -a "$INPUT" != "y" ]; then
        exit 0
    fi
    git tag -a $1 -m "Release $1"
fi

VERSION=`git describe --tags`
if [ -z "$VERSION" ]; then
    VERSION=`git rev-parse --short HEAD`
fi

sed -i "s/^\!define PRODUCT_VERSION.*/\!define PRODUCT_VERSION \"${VERSION}\"/g" ${SOURCE_DIR}/Install/Install.nsi
sed -i "s/^\SET(BUILD_VERSION.*/\SET(BUILD_VERSION \"${VERSION}\")/g" ${SOURCE_DIR}/CMakeLists.txt
sed -i "s/<VERSION>.*</<VERSION>${VERSION}</g" ${SOURCE_DIR}/Update/update.xml
#sed -i "s/version:.*/version: \"${VERSION}.{build}\"/g" ${SOURCE_DIR}/appveyor.yml
#sed -i "s/-m \"v[0-9]\+\.[0-9]\+\.[0-9]\+\"/-m \"${VERSION}\"/g" ${SOURCE_DIR}/appveyor.yml
#sed -i "s/RabbitProxyServer-Setup-v[0-9]\+\.[0-9]\+\.[0-9]\+/RabbitProxyServer-Setup-${VERSION}/g" ${SOURCE_DIR}/appveyor.yml
#sed -i "s/RabbitProxyServer_[0-9]\+\.[0-9]\+\.[0-9]\+/RabbitProxyServer_${VERSION}/g" ${SOURCE_DIR}/appveyor.yml

#sed -i "s/tag_name:.*/tag_name: '${VERSION}'/g" ${SOURCE_DIR}/.github/workflows/cmake_mingw.yml
sed -i "s/RABBITPROXYSERVER_VERSION:.*/RABBITPROXYSERVER_VERSION: '${VERSION}'/g" ${SOURCE_DIR}/.github/workflows/msvc.yml
sed -i "s/-m \"v[0-9]\+\.[0-9]\+\.[0-9]\+\"/-m \"${VERSION}\"/g" ${SOURCE_DIR}/.github/workflows/msvc.yml
#sed -i "s/PROJECT_NUMBER.*v[0-9]\+\.[0-9]\+\.[0-9]\+/PROJECT_NUMBER = ${VERSION}/g" ${SOURCE_DIR}/Doxyfile.in

DEBIAN_VERSION=`echo ${VERSION}|cut -d "v" -f 2`
sed -i "s/^\Standards-Version:.*/\Standards-Version:\"${DEBIAN_VERSION}\"/g" ${SOURCE_DIR}/debian/control
sed -i "s/rabbitproxyserver (.*)/rabbitproxyserver (${DEBIAN_VERSION})/g" ${SOURCE_DIR}/debian/changelog
sed -i "s/RABBITPROXYSERVER_VERSION:.*/RABBITPROXYSERVER_VERSION: '${DEBIAN_VERSION}'/g" ${SOURCE_DIR}/.github/workflows/cmake_ubuntu.yml
sed -i "s/Version=.*/Version=${DEBIAN_VERSION}/g" ${SOURCE_DIR}/debian/RabbitProxyServerConfig.desktop
sed -i "s/[0-9]\+\.[0-9]\+\.[0-9]\+/${DEBIAN_VERSION}/g" ${SOURCE_DIR}/README*.md
#sed -i "s/[0-9]\+\.[0-9]\+\.[0-9]\+/${DEBIAN_VERSION}/g" ${SOURCE_DIR}/App/android/AndroidManifest.xml

MAJOR_VERSION=`echo ${DEBIAN_VERSION}|cut -d "." -f 1`
#sed -i "s/android:versionCode=.*android/android:versionCode=\"${MAJOR_VERSION}\" android/g" ${SOURCE_DIR}/App/android/AndroidManifest.xml

if [ -n "$1" ]; then
    git add .
    git commit -m "Release $1"
    #git push
    git tag -d $1
    git tag -a $1 -m "Release $1"
    #git push origin :refs/tags/$1
    git push origin $1
fi
