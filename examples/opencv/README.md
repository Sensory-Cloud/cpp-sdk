# Video Services using OpenCV

This project uses OpenCV to provide a generic camera interface that will compile
on most major build platforms.

## Requirements

### CMake

This project uses CMake as the primary build system. To install CMake, run the
following installation command depending on your deployment target.

#### Debian

```shell
sudo apt-get install build-essential autoconf libtool pkg-config cmake
```

#### MacOS

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake
```

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

## Usage

To compile the various projects:

```
cmake .
make
```
