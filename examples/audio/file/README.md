# SensoryCloud Audio Services (from files)

This project provides a demonstration of SensoryCloud audio services from files
using libsndfile.

## Compilation

To compile the various projects:

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

### Speech-To-Text (STT)

SensoryCloud STT utilizes an end-to-end architecture that has been designed to
offer high flexibility and accuracy. An optional domain-specific language model
can be customized and applied to support unique domains with special vocabulary
or industry-specific jargon. The platform is suitable for use with both
streaming audio and batch modes. To transcribe speech to text:

```shell
./transcribe <path to config.ini file> \
    -i <path to audio file> \
    -o <path to output text file> \
    -m <transcription model name> \
    -u <user ID> \
    -l <language code>
```

### Text-To-Speech (TTS)

SensoryCloud TTS is based on a combination of end-to-end models and neural
vocoders. The end result is perfectly human sounding sythesized speech that
also runs significantly faster than real-time to minimize synthesis delay.
To synthesize speech from text:

```shell
./synthesize_speech <path to config.ini file> \
    -o <path to output audio file> \
    -l <language code> \
    -V <synthesis voice> \
    -p <text to synthesize into speech> \
```
