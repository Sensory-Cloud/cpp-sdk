#!/bin/bash
# ------------------------------------------------------------------
# [Author] Christian Kauten (ckauten@sensoryinc.com)
# [Title] C++ cloud SDK helper script
# ------------------------------------------------------------------

USAGE="Usage: ./cs.sh [COMMAND]"

COMMANDS="
    Commands:\n
    genproto | gp\t\t Generate Proto Files\n
    test | t\t\t Run Unit Tests\n
    bench | b\t\t Run benchmarks\n
    lint | l\t\t Lint Source Files\n
    doc | d\t\t Generate Documentation\n
    help | h\t\t Display This Help Message\n
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
gen_proto() {
  # Generate a build directory for the generated gRPC code
  PROTOC_OUTPUT_PATH="./build/sensorycloud/protoc"
  mkdir -p ${PROTOC_OUTPUT_PATH}
  # Generate the C++ source for the protobuff files
  for x in $(find ./proto -iname "*.proto");
  do
    protoc \
      -I="src"\
      --proto_path="./proto" \
      --cpp_out=${PROTOC_OUTPUT_PATH} \
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
  rm -rf ${PROTOC_OUTPUT_PATH}
}

compile() {
  cmake .
  make sensorycloud
}

compile_tests() {
  make test_sensorycloud_config
}

compile_benchmarks() {
  echo 'TODO'
}

run_tests() {
  ./build/test_sensorycloud_config
}

run_benchmarks() {
  echo 'TODO'
}

# --- Body ---------------------------------------------------------
case "$1" in

  "lint"|"l")
    make clean
    cmake "-DCMAKE_CXX_CPPLINT=cpplint" .
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

  "genproto"|"gp")
    gen_proto
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
