# SensoryCloud Video Services (Real-time)

This project provides a demonstration of SensoryCloud video services using
OpenCV as the video interface driver.

## Requirements

To install the OpenCV library and header files, run the following.

### Debian

```shell
sudo apt install libopencv-dev
```

### MacOS

```shell
brew install opencv
```

## Compilation

To compile the applications in this project:

```shell
mkdir -p build
cd build
cmake ..
make
```

## Usage

Before getting started you must spin up a [SensoryCloud][sensory-cloud]
inference server or have [SensoryCloud][sensory-cloud] spin one up for you. You
must also have the following pieces of information:

-   your inference server address and port number,
-   your SensoryCloud tenant ID, and
-   your configured secret key used for registering OAuth clients.

[sensory-cloud]: https://sensorycloud.ai/

### Face Recognition

A highly flexible facial recognition solution allows app developers or OEMs to
quickly add face biometrics to any mobile or desktop application or device.
SensoryCloud Face Recognition supports advanced cloud features such as
single-frame liveness, cross-device authentication, and continuous model
updates to verify identity by matching the user’s face to a stored biometric
template translated to an irreversible encrypted code.

#### Enrollment

To fetch video models that support enrollment:

```shell
./enroll <path to config.ini file> -g
```

To enroll without a liveness check:

```shell
./enroll <path to config.ini file> -m <model name> -u <user ID>
    -d "A description of the enrollment"
```

To enroll with a liveness check:

```shell
./enroll <path to config.ini file> -m <model name> -u <user ID> -l -t HIGH \
    -d "A description of the enrollment"
```

#### Authentication

To fetch enrollments for a particular user:

```shell
./authenticate <path to config.ini file> -u <user ID>
```

To authenticate without liveness:

```shell
./authenticate <path to config.ini file> -e <enrollment ID>
```

To authenticate with liveness:

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -l -t HIGH
```

#### Liveness Validation

To fetch available video liveness models:

```shell
./liveness <path to config.ini file> -g
```

To validate video liveness (outside of the context of a biometric enrollment):

```shell
./liveness <path to config.ini file> -m <model name> -u <user ID> -t HIGH
```
