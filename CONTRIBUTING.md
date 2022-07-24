# Development

The following passages articulate the setup of the development environment and
describe how to generate new protobuf & gRPC code, run unit tests, etc.

## Compilation

To compile the library from the top-level of this repository:

```shell
mkdir -p build && cd build
cmake ..
make
```

## Protobuf and gRPC code generation

To generate proto and gRPC code, execute:

```shell
cmake -DSENSORY_CLOUD_GENERATE_PROTO=ON ..
make
```

## Docker image generation

To generate docker images for testing on Linux:

```shell
docker compose build --parallel
```

this will build docker images according to the `docker-compose.yaml` file.

## Unit testing

To build unit tests, execute:

```shell
cmake -DSENSORY_CLOUD_BUILD_TESTS=ON ..
make
```

to execute unite tests, execute:

```shell
find . -name "test_sensorycloud*" -maxdepth 1 -type f -exec echo {} \; -exec {} \;
```

## Code release procedure

This passage outlines the steps involved with releasing a new version of the
SDK.

1.  Pull the latest proto files from the `proto` sub-module.

    ```shell
    cd proto
    git pull
    git checkout <branch or tag>
    cd ..
    git add proto
    git commit -m 'update proto to <branch or tag>'
    ```

1.  Recompile the protobuf and gRPC header and source files.

    ```shell
    mkdir -p build && cd build
    cmake -DSENSORY_CLOUD_GENERATE_PROTO=ON ..
    make
    cd ..
    git add include/sensorycloud/generated/
    git commit -m 'regenerate proto/grpc headers and source for <branch or tag>'
    ```

1.  Run unit tests against all target platforms and confirm a green light.

    ```shell
    mkdir -p build && cd build
    cmake -DSENSORY_CLOUD_BUILD_TESTS=ON ..
    make
    find . -name "test_sensorycloud*" -maxdepth 1 -type f -exec echo {} \; -exec {} \;
    ```

1.  Compile example projects against target platforms using the development
    branch and execute the example applications to test as needed.

    ```shell
    mkdir -p build && cd build
    cmake -DSENSORY_CLOUD_BUILD_EXAMPLES=ON ..
    make
    ```

1.  Update the `URL` in the README snippets and `CMakeLists.txt` of example
    projects (replace `<tag to target>` with the tag for the next release).

    ```cmake
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/<tag to target>
    ```

1.  Update documentation with doxygen.

    ```shell
    doxygen Doxyfile
    ```

1.  Update `CHANGELOG.md` with the new features / bug fixes / changes.
1.  Request a code review from another team member.
1.  Merge changes into the main branch (they will be mirrored to GitHub).
1.  Tag the commit and document the changes in the release notes on GitLab.
1.  Compile example projects against all target platforms using the
    released production code from GitHub.
