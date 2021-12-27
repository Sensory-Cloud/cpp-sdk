#!/bin/bash
# ------------------------------------------------------------------
# [Author] Christian Kauten (ckauten@sensoryinc.com)
# [Title] C++ cloud SDK helper script
# ------------------------------------------------------------------

USAGE="Usage: ./cs.sh [COMMAND]"

COMMANDS="
    Commands:\n
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
  make tests
}

# Compile the benchmark suite.
compile_benchmarks() {
  make benchmarks
}

# Execute the unit test suite.
run_tests() {
  find build -name "test*" -exec echo {} \; -exec {} \;
}

# Execute the benchmark suite.
run_benchmarks() {
  find build -name "benchmark*" -exec echo {} \; -exec {} \;
}

# --- Body ---------------------------------------------------------

case "$1" in

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

  "benchmark"|"b")
    compile
    compile_benchmarks
    run_benchmarks
    exit 0;
    ;;

  "build_images"|"bi")
    docker compose build --parallel
    # docker build -f Dockerfile.Ubuntu.16.04 -t sensory-cloud-cpp-sdk:Ubuntu16.04 .
    # docker build -f Dockerfile.Ubuntu.18.04 -t sensory-cloud-cpp-sdk:Ubuntu18.04 .
    # docker build -f Dockerfile.Ubuntu.20.04 -t sensory-cloud-cpp-sdk:Ubuntu20.04 .
    # docker build -f Dockerfile.Ubuntu.21.10 -t sensory-cloud-cpp-sdk:Ubuntu21.10 .
    # docker build -f Dockerfile.Debian.9 -t sensory-cloud-cpp-sdk:Debian9 .
    # docker build -f Dockerfile.Debian.10 -t sensory-cloud-cpp-sdk:Debian10 .
    # docker build -f Dockerfile.Debian.11 -t sensory-cloud-cpp-sdk:Debian11 .
    # docker build -f Dockerfile.CentOS.7 -t sensory-cloud-cpp-sdk:CentOS7 .
    # docker build -f Dockerfile.CentOS.8 -t sensory-cloud-cpp-sdk:CentOS8 .
    # docker build -f Dockerfile.RHEL.7 -t sensory-cloud-cpp-sdk:RHEL7 .
    # docker build -f Dockerfile.RHEL.8 -t sensory-cloud-cpp-sdk:RHEL8 .
    exit 0;
    ;;

  "test_all"|"ta")
    # docker run --rm -it sensory-cloud-cpp-sdk:Ubuntu16.04 ls cpp-sdk
    # docker run --rm -it sensory-cloud-cpp-sdk:Ubuntu18.04 ls cpp-sdk
    # docker run --rm -it sensory-cloud-cpp-sdk:Ubuntu20.04 ls cpp-sdk
    docker run --rm -it sensory-cloud-cpp-sdk:Ubuntu20.04 ls
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
