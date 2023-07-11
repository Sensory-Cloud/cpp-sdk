# SensoryCloud Audio Services

This project provides a demonstration of SensoryCloud audio services using
portaudio/libsoundfile as the audio interface.

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

-   [Speech-to-text (STT)](#speech-to-text-stt)
    -   [Capitalization and Punctuation](#capitalization-and-punctuation)
    -   [Custom Vocabulary](#custom-vocabulary)
    -   [Wake Word Triggered Transcription](#wake-word-triggered-transcription)
    -   [Single Utterance Mode](#single-utterance-mode)
-   [Text-to-speech (TTS)](#text-to-speech-tts)
-   [Wake Word Verification and Sound ID](#wake-word-verification-and-sound-id)
    -   [Event Validation](#event-validation)
    -   [Event Enrollment](#event-enrollment)
    -   [Enrolled Event Validation](#enrolled-event-validation)
-   [Speaker Identification](#speaker-identification)
    -   [Enrollment](#enrollment)
    -   [Authentication](#authentication)

The default examples employ the portaudio library to interface with audio
devices on your machine in real-time. Please refer to
[Audio File Processing](#audio-file-processing) if you are in need of an
interface for testing our models from audio file inputs instead.

Before you start using this example project, it's necessary to have a
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

### Speech-to-text (STT)

The SensoryCloud speech-to-text (STT) service provides a range of features to
achieve flexible and accurate transcription of speech. These features include:

-   Capitalization and Punctuation (P&C) options for enhanced readability of
    transcribed text.
-   Custom vocabulary support to accurately transcribe domain-specific terms
    and jargon.
-   Wake word triggered transcription, enabling activation of the service by a
    specific phrase or saying, making it suitable for automated assistants.
-   Single utterance mode for capturing a single user's speech, catering to
    applications like chat bots and command-and-control interfaces.

With support for both streaming and offline modes, as well as the
aforementioned features, SensoryCloud STT addresses diverse speech-to-text
requirements across various applications.

To fetch transcription models that are available to your tenant in
SensoryCloud, use the following command. Here `<path to config.ini file>`
should be a path to an INI file on disk that describes your tenant in
SensoryCloud. The `-g` flag instructs the tool to fetch models and print them
without performing a transcription.

```shell
./transcribe <path to config.ini file> -g
```

Once you have identified a model to perform transcription with, the follow
command can be executed to initiate a real-time transcription feed from the
default audio input device on your machine.

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to transcribe the speech with.
-   `<user ID>` describes the unique ID for the user that is initiating the
    transcription.

#### Capitalization and Punctuation

By default, the transcription service will return normalized text without any
punctuation and capitalization (P&C). In cases where text is being rendered
for human consumption, P&C is frequently expected and helps improve the flow
of the text when read. To enable P&C for a transcription stream, the `-cp` flag
can be thrown like in the following example command. This enables a P&C model
specific to the localization of the speech being transcribed, and will result
in the emission of fully punctuated and capitalized text where appropriate.

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -cp
```

#### Custom Vocabulary

The SensoryCloud speech transcription model is designed such that the mapping
between the acoustic signal and text can be dynamically adjusted. This allows
the service to specify custom vocabulary and jargon for transcribing speech
from esoteric domains. This is done by defining a mapping between what
domain-specific terms sound like and what they should be transcribed as. To
enable a custom vocabulary, the follow command may be utilized:

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -CV <custom vocab words> \
    -CVs MEDIUM
```

-   the `-CV` flag controls the input for the custom vocabulary. This should
    be a space-delimited list of word tuples in `replacement,to-replace`
    format where the second word is the word to detect and the first word is
    the word to replace with. For instance, to detect instances of
    `sensory cloud` and map them to the brand `SensoryCloud`, we would use
    `-CV SensoryCloud,'sensory cloud'`. Because `sensory cloud` is itself space
    delimited, we wrap it with single quotes to provide it to the CLI.
-   the `-CVs` flag controls the sensitivity of the model to the custom
    vocabulary. This should be one of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`,
    and will default to `MEDIUM` if not provided. Higher sensitivity levels
    are more likely to result in replacements from the custom vocabulary, but
    may result in inaccurate replacements if set too high. Likewise, if the
    sensitivity is too low, replacements from the custom vocabulary may be
    missed.

#### Wake Word Triggered Transcription

Many use cases, particularly automated assistants, benefit from transcription
services that are triggered by wake-words. This enables the gating of
transcription events by a unique phrase or saying. SensoryCloud supports the
integration of our best-in-class wake-word models with our transcription
service to support this use case. The following command may be dispatched to
initiate a transcription stream that is triggered by a wake-word.

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -Wm <wake-word model> \
    -Ws LOW
```

-   `<wake-word model>` describes the model of the wake-word to detect. Please
    refer to
    [Wake Word Verification and Sound ID](#wake-word-verification-and-sound-id)
    for more information about SensoryCloud wake-words, including tooling for
    determining which wake-word models are available to your tenant.
-   the `-Ws` flag controls the sensitivity of the wake-word model. This
    should be one of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and will default
    to `LOW` if not provided. Higher sensitivity levels are more likely to
    result in detections of the wake-word, but may false fire in some settings.

#### Single Utterance Mode

By default, the transcription service will transcribe speech indefinitely,
which can be useful for online systems, such as real-time conference or meeting
transcription. However, there exist use cases where it's desirable to capture
a single utterance from a user, such as chat bots, command-and-control
interfaces, and interactive voice applications. Single utterance mode enables
the transcription service to wait for an active speaker before transcribing
audio and to terminate the transcription once the user has stopped speaking.
To enable single utterance mode, the following command can be executed:

```shell
./transcribe <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -S -Vs LOW -Vd 1
```

-   the `-S` flag enables single utterance mode. The service will wait for
    active speech and terminate once the user has stopped speaking when this
    flag is present.
-   the `-Vs` flag controls the sensitivity of the voice-activity detector
    (VAD) utilized to determine periods of active speech. This should be one
    of `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and will default to `LOW` if
    not provided. Higher sensitivity levels are more likely to detect speech
    in noisy environments, but may result in false detections in some cases.
-   the `-Vd` flag controls the duration of silence to expect after the
    utterance before terminating the stream. This should be a numeric value
    measured in seconds and will default to `1.0` if not provided. Shorter
    durations can help to reduce response time, but may terminate the
    transcription early if set too low. Longer durations may be useful for
    cases  where users are speaking slowly or with long pauses between words.

### Text-to-speech (TTS)

SensoryCloud TTS utilizes a combination of end-to-end models and neural
vocoders to produce synthesized speech that closely resembles human speech.
The synthesis process is optimized for efficiency, enabling speech to be
generated at a speed faster than real-time, thereby minimizing synthesis
delay.

To fetch speech synthesis models that are available to your tenant in
SensoryCloud, use the following command. Here `<path to config.ini file>`
should be a path to an INI file on disk that describes your tenant in
SensoryCloud. The `-g` flag instructs the tool to fetch models and print them
without performing a transcription.

```shell
./synthesize_speech <path to config.ini file> -g
```

Once you've identified a model to perform speech synthesis with, the following
command may be executed to synthesize audio to a WAV file.

```shell
./synthesize_speech <path to config.ini file> \
    -m <model name> \
    -p "Hello, World!" \
    -o speech.wav \
    -fs 22050
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to synthesize the speech with.
-   the `-p` flag provides the phrase to transcribe, `Hello, World!` in this
    case.
-   the `-o` flag controls the output file that is generated. This value will
    default to `speech.wav` if not provided.
-   the `-fs` flag allows the resampling of synthesized audio to a sample rate
    of your choosing. When not provided the generated audio will default to
    the output sample rate of the model (e.g., 22050Hz in most cases.)

### Wake Word Verification and Sound ID

The SensoryCloud platform offers Wake Word Verification and Sound ID
capabilities. Cloud-based verification significantly improves the performance
of wake word detection by reducing false alarms. Sound ID enhances situational
awareness by detecting and alerting users to concerning sounds such as glass
breaking, babies crying, dogs barking, home security alarms, smoke/CO alarms,
doorbells, knocking, snoring, and more. Both Wake Word Verification and Sound
ID can be accessed through a shared interface, which also supports enrolled
sound event detection.

#### Event Validation

To fetch event validation models, use the following command. Here
`<path to config.ini file>` should be a path to an INI file on disk that
describes your tenant in SensoryCloud. The `-g` flag instructs the tool to
fetch models and print them without performing an enrollment.

```shell
./validate_event <path to config.ini file> -g
```

Once you have identified a model to perform event detection with, the follow
command can be executed to initiate a real-time event detection feed from the
default audio input device on your machine.

```shell
./validate_event <path to config.ini file> \
    -m <model name> \
    -u <user ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to transcribe the speech with.
-   `<user ID>` describes the unique ID for the user that is initiating the
    transcription.

#### Event Enrollment

To fetch event validation models that support enrollment, use the following
command. Here `<path to config.ini file>` should be a path to an INI file on
disk that describes your tenant in SensoryCloud. The `-g` flag instructs the
tool to fetch models and print them without performing an enrollment.

```shell
./enroll_event <path to config.ini file> -g
```

Models that support enrollment can be adapted to unique instances of sounds.
Once you've identified an event detection model that you would like to enroll
with, an enrollment can be created from a real-time microphone input using the
following:

```shell
./enroll_event <path to config.ini file> \
    -m <model name> \
    -n <num occurrences> \
    -d "A description of the event enrollment" \
    -u <user ID>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to enroll the sound with.
-   `<user ID>` describes the unique ID for the user that is creating the
    enrollment.
-   `<num occureences>` describes the number of unique occurrences of the sound
    that are required to generate the enrollment. Larger values typically
    result in more robust recognition. When omitted, a model-specific value
    will be used.
-   The `-d` flag allows one to provide an optional description of the
    enrollment.

#### Enrolled Event Validation

The enrolled event validation service allows users to validate sounds against
existing enrollments. Before utilizing this tool, it's necessary to determine
an existing enrollment to authenticate against. If you've just enrolled a sound
using the enrollment tool, the ID of the enrollment will have been produced by
the tool for your storage and usage. Otherwise, to fetch available sound
enrollments for a particular user, the following command may be used.

```shell
./validate_enrolled_event <path to config.ini file> -u <user ID>
```

This will print a JSON formatted list of each enrollment available for the
given user.

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user to fetch enrollments for. _This value
    corresponds to the same user ID that was provided on enrollment._

Once an enrollment has been identified and its ID noted, one may validate
against the enrollment using the following:

```shell
./validate_enrolled_event <path to config.ini file> -e <enrollment ID> -s LOW
```

-   `<enrollment ID>` is the ID of the enrollment to authenticate against.
-   the `-s` flag controls the sensitivity of the model. This should be one of
    `LOW`, `MEDIUM`, `HIGH`, or `HIGHEST`, and will default to `HIGH` if not
    provided.

### Speaker Identification

SensoryCloud Speaker Recognition provides a highly flexible speaker recognition
solution that allows app developers or OEMs to quickly add voice biometrics to
any mobile or desktop application or device. It supports advanced cloud
features such as liveness, cross-device authentication, and continuous model
updates.

#### Enrollment

To fetch voice models that support enrollment, use the following command. Here
`<path to config.ini file>` should be a path to an INI file on disk that
describes your tenant in SensoryCloud. The `-g` flag instructs the tool to
fetch models and print them without performing an enrollment.

```shell
./enroll <path to config.ini file> -g
```

SensoryCloud offers two forms of voice biometric model. _text-independent_
models encode acoustic signals into identifiable features that are invariant
to the actual words that are spoken. This allows a user's voice to act as a
pass-phrase without needing to recall a particular utterance. _text-dependent_
models do place a requirement on a known utterance or phrase.

```shell
./enroll <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -n <num occurrences> \
    -d "A description of the enrollment" \
    -L <language code>
```

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<model name>` describes the model to enroll the sound with.
-   `<user ID>` describes the unique ID for the user that is creating the
    enrollment.
-   `<num occureences>` describes the number of unique occurrences of the sound
    that are required to generate the enrollment. Larger values typically
    result in more robust recognition. When omitted, a model-specific value
    will be used.
-   The `-d` flag allows one to provide an optional description of the
    enrollment.
-   The `-L` flag describes the localization of the input audio as an
    [IETF BCP 47](https://www.ietf.org/rfc/bcp/bcp47.html) language
    (e.g., en-US for U.S. English)

Our text-independent ones, support active voice liveness features. This
functionality allows you to ensure that audio samples streamed to the cloud
are authentic live samples, and not replayed recordings. The default
functionality is to enroll _without_ a liveness check. To enable liveness on
enrollment, the `-l` flag may be thrown:

```shell
./enroll <path to config.ini file> -l ...
```

#### Authentication

The authentication end-point allows users to validate audio samples against
existing biometric enrollments. Before utilizing this tool, it's necessary to
determine an existing enrollment to authenticate against. If you've just
enrolled a voice using the enrollment tool, the ID of the enrollment will have
been produced by the tool for your storage and usage. Otherwise, to fetch
available voice enrollments for a particular user, the following command may
be used.

```shell
./authenticate <path to config.ini file> -u <user ID>
```

This will print a JSON formatted list of each enrollment available for the
given user.

-   `<path to config.ini file>` is a path to your SensoryCloud config file.
-   `<user ID>` is the ID of the user to fetch enrollments for. _This value
    corresponds to the same user ID that was provided on enrollment._

Once an enrollment has been identified and its ID noted, one may authenticate
against the enrollment using the following:

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -L <language code>
```

-   `<enrollment ID>` is the ID of the enrollment to authenticate against.
-   The `-L` flag describes the localization of the input audio as an
    [IETF BCP 47](https://www.ietf.org/rfc/bcp/bcp47.html) language
    (e.g., en-US for U.S. English)

Just as with enrollment, liveness may optionally be enabled for text-independent enrollments by throwing the `-l` flag.

```shell
./authenticate <path to config.ini file> -l ...
```

##### Group Authentication

Using the SensoryCloud enrollment group feature, our voice recognition solution
can be utilized for 1 to small N lookup operations. This enables powerful
enterprise solutions involving teams of individuals with unique access control
needs. Please refer to our [management](../management) example if you are
unfamiliar with the process of creating an enrollment group from a set of
existing enrollments. Once an enrollment group has been identified and its ID
noted, one may authenticate against the enrollment group similarly to the
1-to-1 authentication use case described above. The updated command below
introduces the `-g` flag to indicate to the tool that the enrollment
ID refers to a group, as opposed to an individual user.

```shell
./authenticate <path to config.ini file> -e <enrollment ID> -g -L <language code>
```

Upon a successful authentication, the specific user ID will also be returned.
This enables both group-based authentication as well as detection of unique
individuals among small groups or teams.

## Audio File Processing

To keep our code examples simple and task focused, we have separated real-time
processing workflows from file-processing based ones. This is because the two
workflows require separate libraries with different interfaces and data flow
patterns. Our file-processing tools enable automated testing of our systems
based on data that reflects the dynamics of your particular use case. In all
cases, the file-processing counterpart for each tool simply appends the suffix
`_file`. and introduces the CLI flag `-i <path to audio file>` for providing a
path to an input file. Audio files are expected to be monophonic (single
channel) in 16-bit signed format (linear PCM) with a sample rate of _16kHz_.
For instance, below illustrates a theoretical example of executing an
transcription from a WAV file:

```shell
./transcribe_file <path to config.ini file> \
    -m <model name> \
    -u <user ID> \
    -i <path to audio file>
```

-   The `-i` flag instructs the tool to stream data from the file at the given
    path `<path to file>`

**N.B.** The only exception is the speech synthesis example, which is only
implemented with a file-processing example and no real-time counterpart.

## Verbose Outputs

If you're curious about the inner workings of the SensoryCloud API and SDK,
verbose output may be enabled for any tool using the `-v` flag. This results
in the emission of each response from the server to the standard output in
JSON format. This can be useful for debugging cases that are otherwise not
obviated by our command line/graphical user interfaces.
