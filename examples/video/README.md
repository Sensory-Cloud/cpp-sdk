# Sensory Cloud Video Services using OpenCV for Video Input

This project uses OpenCV to provide a generic camera interface that will compile
on most major build platforms.

## Requirements

### OpenCV

To install the OpenCV library and header files, run the following.

#### Debian

```shell
sudo apt install libopencv-dev
```

#### MacOS

```shell
brew install opencv
```

## Compilation

To compile the various projects:

```shell
mkdir -p build
cd build
cmake ..
make
```

## Usage

Before getting started, you must spin up a Sensory Cloud inference server or
have Sensory spin one up for you. You must also have the following pieces of
information:

-   Your inference server URL (and port number)
-   Your Sensory Tenant ID (UUID)
-   Your configured secret key used to register OAuth clients

### Authenticate

To fetch enrollments for a particular user:

```shell
./examples/video/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID>
```

To authenticate without liveness:

```shell
./examples/video/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID>
```

To authenticate with liveness:

```shell
./examples/video/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID> \
    -l -t HIGH
```

### Enroll

To fetch enrollable video models:

```shell
./examples/video/build/enroll -g \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

To rnroll without a liveness check:

```shell
./examples/video/build/enroll \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -d "A description of the enrollment"
```

To enroll with a liveness check:

```shell
./examples/video/build/enroll \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -d "A description of the enrollment" \
    -l -t HIGH
```

### Validate Liveness

To fetch available video liveness models:

```shell
./examples/video/build/liveness -g \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

To validate video liveness:

```shell
./examples/video/build/liveness \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -t HIGH
```
