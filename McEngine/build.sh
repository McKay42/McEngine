#!/bin/bash

NAME="McEngine"
BUILD="Linux Release"

SRC="src"
LIB="libraries"

CXX="g++"
CC="gcc"
LD="g++"

CXXFLAGS="-std=c++11 -O3 -Wall -c -fmessage-length=0 -Wno-sign-compare -Wno-unused-local-typedefs -Wno-reorder -Wno-switch"
CFLAGS="-O3 -Wall -c -fmessage-length=0"

LDFLAGS=""
LDFLAGS2=("-static-libstdc++" "-static-libgcc" "-Wl,-rpath=.")
LDLIBS="-ldiscord-rpc -lsteam_api -lcurl -lz -lX11 -lXi -lGL -lGLU -lGLEW -lfreetype -lbass -lbass_fx -lOpenCL -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lenet -lpthread -ljpeg"



STARTTIMESECONDS=$SECONDS

GREEN='\033[0;32m'
RED='\033[0;31m'

FULLPATH=$(dirname "$(readlink -f "$0")")
echo "FULLPATH = $FULLPATH"

echo "Creating $FULLPATH/$BUILD/ directory ..."
rm -rf "$BUILD"
mkdir "$BUILD"

echo "Collecting C++ files ..."
cppfiles=()
while IFS= read -r -d '' cppfile; do
	echo "$cppfile"
	cppfiles+=("$cppfile")
done < <(find "$FULLPATH/$SRC/" -type f -name "*.cpp" -print0)

echo "Collecting C files ..."
cfiles=()
while IFS= read -r -d '' cfile; do
	echo "$cfile"
	cfiles+=("$cfile")
done < <(find "$FULLPATH/$SRC/" -type f -name "*.c" -print0)

echo "Collecting $SRC include paths ..."
includepaths=()
while IFS= read -r -d '' includepath; do
	echo "$includepath"
	includepaths+=("-I$includepath")
done < <(find "$FULLPATH/$SRC/" -type d -print0)

echo "Collecting library include paths ..."
while IFS= read -r -d '' includepath; do
	echo "$includepath"
	includepaths+=("-I$includepath")
done < <(find "$FULLPATH/$LIB/"*/include -maxdepth 0 -type d -print0)

echo "Collecting library search paths ..."
librarysearchpaths=()
while IFS= read -r -d '' librarysearchpath; do
	echo "$librarysearchpath"
	librarysearchpaths+=("-L$librarysearchpath")
done < <(find "$FULLPATH/$LIB/"*/lib/linux -maxdepth 0 -type d -print0)

echo "Compiling ${#cppfiles[@]} C++ file(s) ..."
COUNTER=0
for cppfile in "${cppfiles[@]}"; do
	cppfilename="$(basename -- $cppfile)"
	objectfilename="${cppfilename%.cpp}.o"

	cmd="$CXX $CXXFLAGS ${includepaths[@]/#/} -o \"$FULLPATH/$BUILD/${COUNTER}_cpp_$objectfilename\" \"$cppfile\""
	echo "$cmd"
	$($CXX $CXXFLAGS "${includepaths[@]/#/}" -o "$FULLPATH/$BUILD/${COUNTER}_cpp_$objectfilename" "$cppfile")

	ret=$?
	if [ "$ret" -ne 0 ]; then
		ELAPSEDTIMESECONDS=$(($SECONDS - $STARTTIMESECONDS))
		echo -e "${RED}Build Failed. (took $ELAPSEDTIMESECONDS second(s))"
		exit $ret
	fi
	
	let COUNTER=COUNTER+1
done

echo "Compiling ${#cfiles[@]} C file(s) ..."
COUNTER=0
for cfile in "${cfiles[@]}"; do
	cfilename="$(basename -- $cfile)"
	objectfilename="${cfilename%.c}.o"

	cmd="$CC $CFLAGS -o \"$FULLPATH/$BUILD/${COUNTER}_c_$objectfilename\" \"$cfile\""
	echo "$cmd"
	$($CC $CFLAGS -o "$FULLPATH/$BUILD/${COUNTER}_c_$objectfilename" "$cfile")

	ret=$?
	if [ "$ret" -ne 0 ]; then
		ELAPSEDTIMESECONDS=$(($SECONDS - $STARTTIMESECONDS))
		echo -e "${RED}Build Failed. (took $ELAPSEDTIMESECONDS second(s))"
		exit $ret
	fi
	
	let COUNTER=COUNTER+1
done

echo "Collecting object files ..."
ofiles=()
while IFS= read -r -d '' ofile; do
	echo "$ofile"
	ofiles+=("$ofile")
done < <(find "$FULLPATH/$BUILD/" -type f -name "*.o" -print0)

echo "Linking ${#ofiles[@]} object file(s) ..."
cmd="$LD $LDFLAGS ${LDFLAGS2[@]/#/} ${librarysearchpaths[@]/#/} -o \"$FULLPATH/$BUILD/$NAME\" ${ofiles[@]/#/} $LDLIBS"
echo "$cmd"
$($LD $LDFLAGS "${LDFLAGS2[@]/#/}" "${librarysearchpaths[@]/#/}" -o "$FULLPATH/$BUILD/$NAME" "${ofiles[@]/#/}" $LDLIBS)

ret=$?
if [ "$ret" -ne 0 ]; then
	ELAPSEDTIMESECONDS=$(($SECONDS - $STARTTIMESECONDS))
	echo -e "${RED}Build Failed. (took $ELAPSEDTIMESECONDS second(s))"
	exit $ret
fi

ELAPSEDTIMESECONDS=$(($SECONDS - $STARTTIMESECONDS))
echo -e "${GREEN}Build Finished. (took $ELAPSEDTIMESECONDS second(s))"
exit 0

