#! /usr/bin/env bash

# "PureData.app" automagically built.
# 
#   ./MacOSX.sh
#
# Tested on:
#
#   - Mac OS X 10.6.8 (Snow Leopard)
#

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level"; exit 1; }

# Frameworks ( http://jackaudio.org/downloads/ ).

wish="/System/Library/Frameworks/Tk.framework/Versions/8.5/Resources/Wish.app"
jack="/System/Library/Frameworks/Jackmp.framework/Headers/jack.h"

[ -e "${wish}" ] || { echo >&2 "${0##*/}: cannot find Tk framework"; exit 1; }

# Paths.

folder="${rep}/PureData"
app="${folder}/PureData.app"

plist="${rep}/resources/Info.plist"
bin="${rep}/bin"
tcl="${rep}/tcl"

org="org.puredata.puredata.plist"
default="${rep}/resources/${org}"
preferences="${HOME}/Library/Preferences"

patches="${rep}/resources/patches"

# Do not overwrite previous build.

[ -e "${folder}" ] && { echo >&2 "${0##*/}: ${folder} already exist"; exit 1; }
[ -e "${app}" ]    && { echo >&2 "${0##*/}: ${app} already exist"; exit 1; }

# Architectures.

arch="-arch ${HOSTTYPE}"
echo "Build ${HOSTTYPE} ..."

# Build the binaries (JACK framework is used by default).

cd "${rep}/src"                                                 || exit 1

if [ -e "${jack}" ]; then
    echo "Build with JACK ... "
    make -f makefile.mac "ARCH=${arch}" JACK=TRUE               || exit 1
else
    make -f makefile.mac "ARCH=${arch}"                         || exit 1
fi

cd "${rep}"                                                     || exit 1

# Make the bundle.

echo "Build package ..."

mkdir "${folder}"                                               || exit 1
cp -R "${wish}" "${app}"                                        || exit 1

rm -f "${app}/Contents/Info.plist"                              || exit 1
rm -f "${app}/Contents/PkgInfo"                                 || exit 1
rm -f "${app}/Contents/MacOS/Wish Shell"                        || exit 1
rm -f "${app}/Contents/_CodeSignature/CodeResources"            || exit 1
rmdir "${app}/Contents/_CodeSignature"                          || exit 1
rm -f "${app}/Contents/CodeResources"                           || exit 1

cp -p "${plist}" "${app}/Contents/Info.plist"                   || exit 1
echo "APPL????" > "${app}/Contents/PkgInfo"                     || exit 1
mv "${app}/Contents/MacOS/Wish" "${app}/Contents/MacOS/Pd"      || exit 1
cp -R "${bin}" "${app}/Contents/Resources/"                     || exit 1
cp -R "${tcl}" "${app}/Contents/Resources/"                     || exit 1

cd "${app}/Contents/Resources/"                                 || exit 1
ln -s "tcl" "Scripts"                                           || exit 1
cd "${rep}"                                                     || exit 1

# Install default preferences.

[ -e "${preferences}/${org}" ] || { echo "Install preferences ..."; cp "${default}" "${preferences}"; }

# Install materials.

echo "Install patches ..."

cp -R "${patches}" "${folder}"                                  || exit 1

# Clean the build.

echo "Clean ..."

cd "${rep}/src"                                                 || exit 1
make -f makefile.mac clean                                      || exit 1
cd "${rep}"                                                     || exit 1

rmdir "${bin}"                                                  || exit 1

# End.

echo "SUCCEEDED"
