#include <iostream>
#include <types.h>
#include "options.h"
#include "decoder.h"
#include "player.h"

void printLogo()
{
	std::cout <<
	"asciiplay - ASCII video player\n"
	"Built on " __DATE__ " " __TIME__ "\n"
	<< std::endl;
}

void printHelp()
{
	std::cout <<
	"Usage: asciiplay [options] [input]\n"
	"Options:\n"
	"    -h, --help: display this help message\n"
	"    -info: show video info before playing\n"
	"    -disablemouse: disable mouse input\n"
	"    -disablekb: disable keyboard input\n"
	"    -disableui: disable the user interface"
	"    \n"
	<< std::endl;
}

void showInfo(Video &video)
{
	bool hasAudio = video.audio != nullptr;
	std::cout << "Video info\n"
	             "-----------------------------\n"
	             "Width: " << video.width << "\n"
				 "Height: " << video.height << "\n"
				 "FPS: " << video.fps << "\n"
				 "Frame count: " << video.frames.size() << "\n"
				 "Character count: " << video.charset.size() << "\n"
				 "Has audio: " << (hasAudio ? "Yes" : "No") << "\n";
	if (hasAudio)
	{
		Audio* audio = video.audio;
		std::string format;
		switch (audio->format)
		{
		case Audio::WAV:
			format = "WAV";
			break;
		case Audio::FLAC:
			format = "FLAC";
			break;
		case Audio::MP3:
			format = "MP3";
			break;
		case Audio::OGG:
			format = "OGG (Vorbis)";
			break;
		}
		std::cout << "Audio format: " << format << "\n";
	}
	std::cout << "\n";
	std::cout << "Hit Enter to start playing..." << std::endl;
	std::cin.ignore();
}

int main(int argc, char** argv)
{
	printLogo();

	Options opts(argc, argv);

	if (opts.help) {
		printHelp();
		return 0;
	}

	Video video;
	if (!opts.input.empty()) {
		// decode the video
		bool ret = Decoder::decodeFile(opts.input, video);
		if (!ret) {
			std::cout << "Decoding failed, aborting." << std::endl;
			return 1;
		}

		if (opts.info)
			showInfo(video);
	}

	// initialize the player
	Player player(video, opts);
	if (!player.init())
	{
		std::cout << "Player initialization failed, exiting." << std::endl;
		return 1;
	}

	// enter the main loop
	player.main();

	return 0;
}
