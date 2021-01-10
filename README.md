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
exactly the same as Linux as described above.
# Demo
As usual, here's the classic [Bad Apple!!](https://youtu.be/C_qwSUhRv8U)
