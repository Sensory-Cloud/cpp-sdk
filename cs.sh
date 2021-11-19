#!/bin/bash
# ------------------------------------------------------------------
# [Author] Christian Kauten (ckauten@sensoryinc.com)
# [Title] C++ cloud SDK helper script
# ------------------------------------------------------------------

USAGE="Usage: ./cs.sh [COMMAND]"

COMMANDS="
    Commands:\n
    genproto | gp\t\t Compile Proto Files\n
    compile | c\t\t Compile the library\n
    lint | l\t\t Lint source files\n
    test | t\t\t Run unit tests\n
    bench | b\t\t Run benchmarks\n
    doc | d\t\t Generate documentation\n
    help | h\t\t Display this help message\n
"

# The directory where the header files are stored
INCLUDE_DIR="./include/sensorycloud"

# The directory where the definition files are stored
SRC_DIR="./src/sensorycloud"

print_helper() {
  echo
  echo ${USAGE}
  echo
  echo -e ${COMMANDS}
}

# --- Options Processing -------------------------------------------

if [ $# == 0 ] ; then
    print_helper
    exit 1;
fi

# --- Helper Functions ---------------------------------------------

# Generate the protobuf C++ headers and definitions
gen_proto() {
  # Generate a build directory for the generated gRPC code
  PROTOC_OUTPUT_PATH="./build/sensorycloud/protoc"
  rm -rf ${PROTOC_OUTPUT_PATH}
  mkdir -p ${PROTOC_OUTPUT_PATH}
  # Generate the C++ source for the protobuff files
  for x in $(find ./proto -iname "*.proto");
  do
    protoc \
      --proto_path="./proto" \
      --cpp_out=${PROTOC_OUTPUT_PATH} \
      --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
      --grpc_out=${PROTOC_OUTPUT_PATH} \
      $x;
    echo "Generated grpc code for $x";
  done
  # Iterate over the built files and fix the include paths
  for x in $(find ${PROTOC_OUTPUT_PATH} -name "*.h" -o -name "*.cc");
  do
    sed -i '' "s/#include \"/#include \"sensorycloud\/protoc\//g" $x
    echo "Replace includes $x";
  done
  # Move header files into the include directory
  mkdir -p ${INCLUDE_DIR}
  rm -rf "${INCLUDE_DIR}/protoc"
  cp -r ${PROTOC_OUTPUT_PATH} "${INCLUDE_DIR}/protoc"
  find "${INCLUDE_DIR}/protoc" -name "*.cc" -delete
  # Move definition files into the src directory
  mkdir -p ${SRC_DIR}
  rm -rf "${SRC_DIR}/protoc"
  cp -r ${PROTOC_OUTPUT_PATH} "${SRC_DIR}/protoc"
  find "${SRC_DIR}/protoc" -name "*.h" -delete
  # Remove the build directory
  # rm -rf ${PROTOC_OUTPUT_PATH}
}

# Compile the Makefiles and library. Also pull some dependencies.
compile() {
  cmake .
  make sensorycloud
}

# Lint the header files and definition files.
lint() {
  make clean
  cmake "-DCMAKE_CXX_CPPLINT=cpplint" .
  make
  make clean
  rm CMakeCache.txt
  rm -rf CMakeFiles
}

# Compile the unit test suite.
compile_tests() {
  make test_sensorycloud_config
}

# Compile the benchmark suite.
compile_benchmarks() {
  echo 'TODO'
}

# Execute the unit test suite.
run_tests() {
  ./build/test_sensorycloud_config
}

# Execute the benchmark suite.
run_benchmarks() {
  echo 'TODO'
}

# --- Body ---------------------------------------------------------

case "$1" in

  "genproto"|"gp")
    gen_proto
    exit 0;
    ;;

  "compile"|"c")
    compile
    exit 0;
    ;;

  "lint"|"l")
    lint
    exit 0;
    ;;

  "test"|"t")
    compile
    compile_tests
    run_tests
    exit 0;
    ;;

  "bench"|"b")
    compile
    compile_benchmarks
    run_benchmarks
    exit 0;
    ;;

  "doc"|"d")
    doxygen Doxyfile
    exit 0;
    ;;

  "help"|"h")
    print_helper
    exit 0;
    ;;

  *)
    print_helper
    exit 1;
    ;;

esac
