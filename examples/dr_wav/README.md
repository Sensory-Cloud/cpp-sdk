# Audio Services based on File I/O

This project uses [dr_wav][dr_wav], [dr_flac][dr_flac], and [dr_mp3][dr_mp3] to
provide an audio file interface that will compile on most major build platforms.

[dr_wav]: https://mackron.github.io/dr_wav
[dr_flac]: https://mackron.github.io/dr_flac
[dr_mp3]: https://github.com/mackron/dr_libs/blob/master/dr_mp3.h

## Usage

To compile the various projects:

```
mkdir -p build
cd build
cmake ..
make
```
