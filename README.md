# AsciiVid
AsciiVid is an ASCII video codec and player. Just something I made for fun.
# Disclaimer
This project is unfinished. It was only meant to be a [curses](https://en.wikipedia.org/wiki/Curses_(programming_library)) test
but I guess I went overboard :-)

But as a result, there are many bugs and quite a few features are missing.
For instance, the encoder can only encode from PNG files, using them as
separate frames. Or the player using the same framebuffer to draw UI elements
and the actual video frames, leading to several issues. I won't be updating
this but if you're willing to fix those for me, hit me up with a pull request.
# Download
Grab a prebuilt binary over at the [releases](releases) page
or compile it yourself using the instructions below.
# Compiling
## Linux
Make sure to have some common development tools and a curses library installed.

Run these commands in a terminal:
```bash
git clone https://github.com/LeadRDRK/AsciiVid.git
cd AsciiVid
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
## Windows
Using a Unix-like environment with MinGW-w64 is recommended. The procedure is
exactly the same as Linux as described above. As for the curses library, PDCurses
with wincon is recommended.
# Usage
Example: Encoding a video from *.png files with audio
```
asciienc -i path/to/frames_folder -ifmt png -f 30 -a audio.ogg output.asv
```
Example: Play a video
```
asciiplay video.asv
```
More information can found by executing the programs with the --help option.

Please note that the user interface for asciiplay is incomplete and buggy. It can be disabled using the -disableui option.
# Demo
As usual, here's the classic [Bad Apple!!](https://youtu.be/C_qwSUhRv8U)
