# Audio Services using PortAudio

This project uses PortAudio to provide a generic microphone interface that will
compile on most major build platforms.

## Requirements

This project uses CMake as the primary build system. To install CMake, run the
following installation command depending on your deployment target.

### Debian

```shell
sudo apt-get install build-essential autoconf libtool pkg-config cmake
```

### MacOS

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake
```

## Usage

To compile the various projects:

```
cmake .
make
```
