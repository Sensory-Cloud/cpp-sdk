# SensoryCloud Audio Services (Real-time)

This project provides a demonstration of SensoryCloud audio services using
PortAudio as the audio interface driver.

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

### Speech-to-text (STT)

SensoryCloud STT utilizes an end-to-end architecture that has been designed to
offer high flexibility and accuracy. An optional domain-specific language model
can be customized and applied to support unique domains with special vocabulary
or industry-specific jargon. The platform is suitable for use with both
streaming audio and batch modes. To transcribe speech to text:

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -L <language code>
```

<!-- ### Text-to-speech (TTS)

SensoryCloud TTS is based on a combination of end-to-end models and neural
vocoders. The end result is perfectly human sounding sythesized speech that
also runs significantly faster than real-time to minimize synthesis delay.
To synthesize speech from text:

```shell
./synthesize \
    -m <model name> \
    -u <user ID> \
    -L <language code>
    -t "Hello, World!"
``` -->

### Speaker Identification

Speaker identification automatically identifies and authenticates users based
on their voice. The technology offers support for multiple enrolled users and
enables brands to deliver new experiences where a device instantly associates a
user’s voice with a profile, allowing it to access specific data, track
conversations, or control access to features and capabilities.

#### Enrollment

To fetch available biometric voice models:

```shell
./enroll <path to config.ini file> -g
```

To create an enrollment without an active liveness check using a wakeword:

```shell
./enroll <path to config.ini file> -u <user ID> \
    -m <model name> -n <num utterances> -L <language code> \
    -d "A description of the enrollment"
```

To create an enrollment without an active liveness check using a
text-independent model:

```shell
./enroll <path to config.ini file> -u <user ID> \
    -m <model name> -d <max duration> -L <language code> \
    -d "A description of the enrollment"
```

To create an enrollment with an active liveness check using a text-independent
model:

```shell
./enroll <path to config.ini file> -u <user ID> \
    -m <model name> -l -L <language code> \
    -d "A description of the enrollment"
```

#### Authentication

To fetch biometric voice enrollments for a user ID:

```shell
./authenticate <path to config.ini file> -u <user ID>
```

To authenticate without a liveness check:

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -L <language code>
```

To authenticate with a liveness check:

```shell
./authenticate <path to config.ini file> -e <enrollment ID> \
    -l -s HIGH -t HIGH -L <language code>
```

### Wake Word Verification and Sound ID

Most wake words live on the edge, but cloud-based verification can
significantly improve wake word performance. Enabling verification in the cloud
is a valuable technique for reducing false alarms. The wake word data can also
be used to automatically train new edge-based models for ongoing improvements.
Similarly, Sound ID makes devices cognizant of concerning sounds and can warn
people when they occur to enhance situational awareness at home, at work and
more. Our models are trained to recognize a variety of environmental sounds,
including glass breaking, babies crying, dogs barking, home security alarms,
smoke/CO alarms, doorbells, knocking, snoring and more. SensoryCloud provides
both of these technologies using a shared interface with support for enrolled
event detection.

#### Event Validation

To fetch the available event validation models:

```shell
./validate_event <path to config.ini file> -g
```

To detect a sound event like a wake-word or a dog bark:

```shell
./validate_event <path to config.ini file> -u <user ID> \
    -m <model name> -L <language code>
```

#### Event Enrollment

To fetch the available enrollable event validation models:

```shell
./enroll_event <path to config.ini file> -g
```

To create an enrolled event:

```shell
./enroll_event <path to config.ini file> -u <user ID> \
    -m <model name> -n <num occurrences> -L <language code> \
    -d "A description of the event enrollment"
```

#### Enrolled Event Validation

To fetch enrolled events belonging to a particular user:

```shell
./validate_enrolled_event <path to config.ini file> -u <user ID>
```

To validate an enrolled event:

```shell
./validate_enrolled_event <path to config.ini file> -e <enrollment ID> -s LOW
```
