# Video Services using OpenCV

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

## Usage

To compile the various projects:

```
mkdir -p build
cd build
cmake ..
make
```
