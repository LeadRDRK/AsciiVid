#include "options.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

#define HELP 1
#define INFO 2
#define DISABLEMOUSE 3
#define DISABLEKB 4
#define DISABLEUI 5

static std::unordered_map<std::string, int> optionsMap {
	{"-h",            HELP},
	{"--help",        HELP},
	{"-info",         INFO},
	{"-disablemouse", DISABLEMOUSE},
	{"-disablekb",    DISABLEKB},
	{"-disableui",    DISABLEUI}
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
			input = option;
			break;
		}

		int optionType;
		try {
			optionType = optionsMap.at(option);
		} catch (int e) {
			std::cout << "Error: Invalid option: " << option << "\n\n";
			bad = true;
			break;
		}

		switch (optionType) {
		case HELP:
			help = true;
			break;
		case INFO:
			info = true;
			break;
		case DISABLEMOUSE:
			disableMouse = true;
			break;
		case DISABLEKB:
			disableKb = true;
			break;
		case DISABLEUI:
			disableUi = true;
			break;
		}
	}
}
