#include "options.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

#define HELP 1
#define INPUT 2
#define AUDIO 3
#define CHARSET 4
#define FPS 5
#define SX 6
#define SY 7
#define IFMT 8
#define OFMT 9
#define LISTFORMATS 50

static std::unordered_map<std::string, int> optionsMap {
	{"-h",        HELP},
	{"--help",    HELP},
	{"-i",        INPUT},
	{"--input",   INPUT},
	{"-a",        AUDIO},
	{"--audio",   AUDIO},
	{"-c",        CHARSET},
	{"--charset", CHARSET},
	{"-f",        FPS},
	{"--fps",     FPS},
	{"-sx",       SX},
	{"--xscale",  SX},
	{"-sy",       SY},
	{"--yscale",  SY},
	{"-ifmt",     IFMT},
	{"-ofmt",     OFMT},

	// others
	{"-formats",  LISTFORMATS}
};

static CharacterSet defaultCharset {
	' ', '.', ',', '-', '~', ':', ';', '=', '!', '*', '#', '$', '@'
};

Options::Options(int &argc, char** &argv)
{
	if (argc < 2) {
		bad = true;
		return;
	}
	for (int i = 1; i < argc; i++) {
		if (bad) break;
		std::string option(argv[i]);

		if (i == argc - 1) {
			output = option;
			break;
		}

		auto it = optionsMap.find(option);
		if (it == optionsMap.end()) {
			std::cout << "Error: Invalid option: " << option << "\n\n";
			bad = true;
			break;
		}
        int optionType = it->second;

		if (optionType >= INPUT && optionType <= OFMT) {
			// options within this range requires an argument
			if (i == argc - 1) {
				std::cout << "Error: Option requires an argument.\n\n";
				bad = true;
				break;
			}
		}

		switch (optionType) {
		case HELP:
			help = true;
			break;
		case INPUT:
			input = argv[++i];
			break;
		case AUDIO:
			audio = argv[++i];
			break;
		case CHARSET:
		{
			auto charsetStr = argv[++i];
			for (int num = 0; num < strlen(charsetStr); num++)
			{
				char c = charsetStr[num];
				charset.push_back(c);
			}
			break;
		}
		case IFMT:
			ifmt = argv[++i];
			if (ifmt.rfind(".", 0) != 0)
				ifmt = "." + ifmt;
			break;
		case OFMT:
			ofmt = argv[++i];
			if (ofmt.rfind(".", 0) != 0)
				ofmt = "." + ofmt;
			break;
		case FPS:
			fps = atof(argv[++i]);
			if (fps <= 0) {
				std::cout << "Error: Invalid FPS value.\n\n";
				bad = true;
			}
			break;
		case SX:
			sx = atof(argv[++i]);
			if (sx <= 0) {
				std::cout << "Error: Invalid X-axis scaling value.\n\n";
				bad = true;
			}
			break;
		case SY:
			sy = atof(argv[++i]);
			if (sy <= 0) {
				std::cout << "Error: Invalid Y-axis scaling value.\n\n";
				bad = true;
			}
			break;
		case LISTFORMATS:
			listFormats = true;
			break;
		default:
			std::cout << "Invalid option: " << option << "\n\n";
			bad = true;
			break;
		}
	}
	// set the charset to the default if it's not specified or empty
	if (charset.empty()) charset = defaultCharset;
}
