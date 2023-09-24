BUILD_DIR="./build"
CC="clang"
CFLAGS="-Wall -Wextra -ggdb -Iinclude"
LDFLAGS="-luser32 -lkernel32 -lgdi32 -L$BUILD_DIR -lzzz"

echo "Building Example: Triangle"
$CC $CFLAGS -o $BUILD_DIR/triangle.exe ./examples/triangle.c $LDFLAGS

