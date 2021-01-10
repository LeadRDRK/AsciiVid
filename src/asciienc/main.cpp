#include <iostream>
#include <fstream>
#include "audioloader.h"
#include "writer.h"
#include "encoder.h"
#include "options.h"
#include "types.h"

void printLogo()
{
	std::cout <<
	"asciienc - ASCII video/image encoder\n"
	"Built on " __DATE__ " " __TIME__ "\n"
	<< std::endl;
}

void printHelp()
{
	std::cout <<
	"Usage: asciienc [options] [output]\n"
	"Options:\n"
	"    -h, --help: display this help message\n"
	"    -i, --input [file]: specify an input file\n"
	"    If the path provided is a directory, it will search for\n"
	"    numbered frame images in that directory (1.png, 2.png ...)\n"
	"    ifmt must used and the frames must be in the same format.\n"
	"    -a, --audio [file]: specify an audio file\n"
	"    -c, --charset [characters]: specify a custom character set\n"
	"    -f, --fps [fps]: set/override the fps\n"
	"    -sx, --xscale [num]: set the x-axis (horizontal) scale factor\n"
	"    -sy, --yscale [num]: set the y-axis (vertical) scale factor\n"
	"    Note: The default value of this is 0.5 due to the fact that\n"
	"    fonts's height is usually double the width.\n"
	"    -ifmt [format]: (explicitly) specify the input file format\n"
	"    -ofmt [format]: (explicitly) specify the output file format\n"
	"    -formats: show a list of supported file formats\n"
	<< std::endl;
}

void listFormats()
{
	std::cout <<
	"Available formats:\n"
	"\n"
	"Input:\n"
	"    png    Portable Network Graphics\n"
	"\n"
	"Output:\n"
	"    avf    Ascii Video File\n"
	"    txt    Plain Text File\n"
	<< std::endl;
}

int main(int argc, char** argv)
{
	printLogo();

	Options opts(argc, argv);
	if (opts.bad) {
		printHelp();
		return 1;
	}

	if (opts.help) {
		printHelp();
		return 0;
	}

	if (opts.listFormats) {
		listFormats();
		return 0;
	}

	if (opts.input.empty()) {
		std::cout << "Error: An input file must be specified.\n\n";
		printHelp();
		return 1;
	}

	// create the video struct
	Video video {
		opts.charset, opts.sx, opts.sy, opts.fps
	};

	// encode the frames
	bool ret = Encoder::encodeFrames(opts.input, opts.ifmt, video);
	if (!ret) {
		std::cout << "Encoding failed, aborting." << std::endl;
		return 1;
	}

	// load the audio (if provided)
	if (!opts.audio.empty()) {
		Audio* audio = new Audio;
		if (!AudioLoader::loadAudio(opts.audio, audio))
		{
			std::cout << "Audio loading failed, aborting." << std::endl;
			return 1;
		}
		video.audio = audio;
	}

	// write the file
	if (!Writer::WriteFile(video, opts.output, opts.ofmt))
		return 1;

	// free the audio
	if (video.audio)
		delete[] video.audio;

	return 0;
}
