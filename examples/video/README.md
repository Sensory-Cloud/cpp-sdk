# SensoryCloud Video Services

This project provides a demonstration of SensoryCloud video services using
OpenCV as the video interface driver.

## Requirements

Before getting started, ensure that you have the OpenCV library and header
files installed on your system.

### Debian

To install the OpenCV library and header files on Debian-based systems, run
the following command:

```shell
sudo apt install libopencv-dev
```

### MacOS

To install the OpenCV library and header files on macOS using Homebrew, run
the following command:

```shell
brew install opencv
```

## Compilation

To compile the applications in this project, follow these steps:

```shell
mkdir -p build
cd build
cmake ..
make
```

## Usage

This project contains examples for the following SensoryCloud services:

-   [Face Recognition](#face-recognition)
    -   [Enrollment](#enrollment)
    -   [Authentication](#authentication)
-   [Face Liveness Verification](#face-liveness-verification)

Before you start using this example project, it's important to have a
SensoryCloud inference server set up. If you're new to SensoryCloud, we offer
a free trial server that allows you to test our cloud platform and determine
its suitability for your product. To learn more about deploying an inference
server with SensoryCloud, please visit the [SensoryCloud website][trial].

Once your server is up and running, you'll need the following information to
effectively interact with it using this SDK:

-   The address and port number of your inference server,
-   Your SensoryCloud tenant ID, and
-   Your configured secret key used for registering OAuth clients.

If you have any questions or need assistance with server setup or
configuration, please don't hesitate to [contact our sales team][sales]. We
are here to help ensure a smooth and successful integration of SensoryCloud
into your product.

[trial]: https://sensorycloud.ai/free-credits/
[sales]: https://sensorycloud.ai/resources/contact-us/

### Face Recognition

SensoryCloud Face Recognition provides a highly flexible facial recognition
solution that allows app developers or OEMs to quickly add face biometrics to
any mobile or desktop application or device. It supports advanced cloud
features such as single-frame liveness, cross-device authentication, and
continuous model updates to verify identity by matching the user’s face to a
cryptographically stored biometric template.

#### Enrollment

To fetch video models that support enrollment, use the following command. Here
`<path to config.ini file>` should be a path to an INI file on disk that
describes your tenant in SensoryCloud. The `-g` flag instructs the tool to
fetch models and print them without performing an enrollment.

```shell
./enroll <path to config.ini file> -g
```

Once you've identified a model to enroll with, there are two ways to create an
enrollment depending on whether you wish to use our face liveness verification
filter. The single-frame face liveness model allows you to ensure that images
streamed to the cloud are authentic live samples instead of photos or replays
from screens. The default functionality is to enroll _without_ a liveness
check. This can be done by executing the following:

```shell
./enroll <path to config.ini file> -m <model name> -u <user ID> \
    -d "A description of the enrollment"
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to enroll the user with.
-   `<user ID>` describes the unique ID for the user that is creating the
    enrollment.
-   The `-d` flag allows one to provide an optional description of the
    enrollment.

When using our face liveness feature, the sensitivity of the model may be
adjusted to fit your use cases. The command below illustrates how to conduct
an enrollment with a liveness check:

```shell
./enroll <path to config.ini file> -m <model name> -u <user ID> \
    -l -t HIGH -lN 0 \
    -d "A description of the enrollment"
```

-   The `-l` flag instructs the tool to perform the liveness check.
-   The `-t HIGH` flag provides the threshold level for making liveness
    decisions. This should be one of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and
    will default to `HIGH` if not provided. Higher security levels are less
    likely to produce false accepts, but may introduce higher false reject
    rates depending on the dynamics of your application.
-   The `-lN` flag controls the number of frames that must be considered live
    before the enrollment can be validated. By default, this value is 0 (nil),
    which requires _all_ frames to be live for the enrollment to succeed. If
    the value is 5, for instance, then 5 of the enrolled frames must be marked
    as live before the enrollment can succeed.

#### Authentication

The authentication tools allows users to authenticate users against existing
enrollments. Before authenticating, it's necessary to determine an existing
enrollment to authenticate against.  To fetch enrollments for a particular
user, the following command may be used.

```shell
./authenticate <path to config.ini file> -u <user ID>
```

This will print a JSON formatted list of each enrollment available for the
given user.

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user to fetch enrollments for. This value
    corresponds to the same user ID that was provided on enrollment.

Once an enrollment has been identified and its ID noted, one may authenticate
against the enrollment using the following:

```shell
./authenticate <path to config.ini file> -e <enrollment ID>
```

-   `<enrollment ID>` is the ID of the enrollment to authenticate against.

Just as with enrollment, liveness may optionally be enabled using the
following adjusted command. It is worth noting that when liveness is enabled,
a frame must both match the enrollment template _and_ be considered live to
trigger a successful authentication. This differs from enrollment where `N`
frames in the session must be considered live to successfully enroll.

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -l -t HIGH
```

-   The `-l` flag instructs the tool to perform the liveness check.
-   The `-t HIGH` flag provides the threshold level for making liveness
    decisions. This should be one of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and
    will default to `HIGH` if not provided. Higher security levels are less
    likely to produce false accepts, but may introduce higher false reject
    rates depending on the dynamics of your application.

##### Group Authentication

Using the SensoryCloud enrollment group feature, our face recognition solution
can be utilized for 1 to small N lookup operations. This enables powerful
enterprise solutions involving teams of individuals with unique access control
needs. Please refer to our [management](../management) example if you are
unfamiliar with the process of creating an enrollment group from a set of
existing enrollments. Once an enrollment group has been identified and its ID
noted, one may authenticate against the enrollment group similarly to the
1-to-1 use case. The updated command below introduces the `-g` flag to
indicate to the tool that the enrollment  ID refers to a group, as opposed to
an individual user.

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -g
```

Upon a successful authentication, the specific user ID will also be returned.
This enables detection of unique individuals among small groups or teams.

### Face Liveness Verification

The single-frame face liveness model allows you to ensure that images are
authentic live samples instead of photos or replays from screens. Our face
liveness system can be used in conjunction with our face recognition features
(please see above,) or in isolation for use cases that don't require biometric
authentication. To fetch the face liveness models that are available, use the
following command. Here `<path to config.ini file>` should be a path to an INI
file on disk that describes your tenant in SensoryCloud. The `-g` flag
instructs the tool to fetch models and print them without performing a
liveness check.

```shell
./liveness <path to config.ini file> -g
```

Once you've identified a model to verify face liveness with, the following
command may be executed to conduct a verification session:

```shell
./liveness <path to config.ini file> -m <model name> -u <user ID> -t HIGH
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to verify face liveness with.
-   `<user ID>` is the ID of the user to performing liveness verification.
-   The `-t HIGH` flag provides the threshold level for making liveness
    decisions. This should be one of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and
    will default to `HIGH` if not provided. Higher security levels are less
    likely to produce false accepts, but may introduce higher false reject
    rates depending on the dynamics of your application.

## Image and Video File Processing

Each of our video tools support still-frame image and video file inputs in
addition to real-time feeds from cameras. This enables automated testing of
our systems based on data that reflects the dynamics of your particular use
case. Below illustrates a theoretical example of executing an authentication
from a video file:

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -C <path to file>
```

-   The `-C` flag instructs the tool to stream data from the file at the given
    path `<path to file>`

It is worth noting that this same argument may be used to select among the
various camera sensors detected by OpenCV. By default the value is set to `0`
to select the primary device, but it may be set to any integer index of a
device on your system.

## Verbose Outputs

If you're curious about the inner workings of the SensoryCloud API and SDK,
verbose output may be enabled for any tool using the `-v` flag. This results
in the emission of each response from the server to the standard output in
JSON format. This can be useful for debugging cases that are otherwise not
obviated by our command line/graphical user interfaces.
