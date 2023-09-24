CC="clang"
AR="llvm-ar"
CFLAGS="-Wall -Wextra -ggdb -Iinclude"
LDFLAGS="-luser32 -lkernel32 -lgdi32"
BUILD_DIR="./build"

OBJ_DIR="$BUILD_DIR/obj"
NAME="zzz"

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi
if [ ! -d $OBJ_DIR ]; then
    mkdir $OBJ_DIR
fi

echo "Generating object files"
$CC $CFLAGS -o $OBJ_DIR/zzz_platform_win32.o -c ./src/zzz_platform_win32.c
$CC $CFLAGS -o $OBJ_DIR/zzz_event.o -c ./src/zzz_event.c

echo "Linking stage: Static Library"
$AR rcs $BUILD_DIR/"$NAME.lib" $OBJ_DIR/zzz_event.o $OBJ_DIR/zzz_platform_win32.o
