# Sensory Cloud Audio Services using PortAudio for Audio Input

This project uses PortAudio to provide a generic microphone interface that will
compile on most major build platforms.

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

### Transcribe

To transcribe speech to text:

```shell
./examples/audio/build/transcribe \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -L <language code>
```

### Validate Event

To fetch the available event validation models:

```shell
./examples/audio/build/wakeword -g \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

To validate an event, i.e., a voice event (wakeword):

```shell
./examples/audio/build/validate_event \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -L <language code>
```

To validate an event, i.e., a sound event:

```shell
./examples/audio/build/validate_event \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> \
    -u <user ID> \
    -L <language code>
```

### Enroll

To fetch available biometric voice models:

```shell
./examples/audio/build/enroll -g \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

To create an enrollment without an active liveness check using a wakeword:

```shell
./examples/audio/build/enroll \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> -n <num utterances> \
    -u <user ID> \
    -d "A description of the enrollment" \
    -L <language code>
```

To create an enrollment without an active liveness check using a
text-independent model:

```shell
./examples/audio/build/enroll \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> -d <max duration> \
    -u <user ID> \
    -d "A description of the enrollment" \
    -L <language code>
```

To create an enrollment with an active liveness check using a text-independent
model:

```shell
./examples/audio/build/enroll \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> -l \
    -u <user ID> \
    -d "A description of the enrollment" \
    -L <language code>
```

### Authenticate

To fetch biometric voice enrollments for a user ID:

```shell
./examples/audio/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID>
```

To authenticate without a liveness check:

```shell
./examples/audio/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID> \
    -L <language code>
```

To authenticate with a liveness check:

```shell
./examples/audio/build/authenticate \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID> \
    -l -s HIGH -t HIGH \
    -L <language code>
```

### Enroll Event

To fetch available audio event models:

```shell
./examples/audio/build/enroll_event -g \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID>
```

To create an enrolled event:

```shell
./examples/audio/build/enroll_event \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -m <model name> -n <num occurrences> \
    -u <user ID> \
    -d "A description of the event enrollment" \
    -L <language code>
```

### Validate Enrolled Event

To fetch enrolled events for a particular user:

```shell
./examples/audio/build/validate_enrolled_event \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -u <user ID>
```

To validate an enrolled event

```shell
./examples/audio/build/validate_enrolled_event \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -e <enrollment ID> \
    -s LOW
```
