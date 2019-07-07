ROOT_DIR=$(pwd)
#CONFIGS="Debug Release"
CONFIGS="Debug"

for CONFIG in $CONFIGS; do
    BUILD_DIR=$ROOT_DIR/build/$CONFIG
    BIN_DIR=$ROOT_DIR/bin/$CONFIG

    mkdir -p $BUILD_DIR $BIN_DIR

    cd $BUILD_DIR

    cmake -DCMAKE_BUILD_TYPE=$CONFIG $ROOT_DIR -G "Unix Makefiles" || exit 1

    make
done
