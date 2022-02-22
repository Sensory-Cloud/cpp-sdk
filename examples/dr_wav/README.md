# Sensory Cloud Audio Services using Files for Audio Input

This project uses [dr_wav][dr_wav], [dr_flac][dr_flac], and [dr_mp3][dr_mp3] to
provide an audio file interface that will compile on most major build platforms.

[dr_wav]: https://mackron.github.io/dr_wav
[dr_flac]: https://mackron.github.io/dr_flac
[dr_mp3]: https://github.com/mackron/dr_libs/blob/master/dr_mp3.h

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

### Transcription

To transcribe audio from an MP3, WAV, or FLAC file:

```shell
./transcribe \
    -H <inference server URL> \
    -P <inference server port> \
    -T <tenant ID> \
    -i <path to WAV, MP3, or FLAC file> \
    -o <path to output text file> \
    -m <transcription model name> \
    -u <user ID> \
    -l <language code>
```
