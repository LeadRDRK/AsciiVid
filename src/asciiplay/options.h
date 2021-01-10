#pragma once
#include <types.h>
#include <string>

struct Options {
	Options(int &argc, char** &argv);
	bool help = false;
	std::string input;
	bool info = false;
	bool disableMouse = false;
	bool disableKb = false;
	bool disableUi = false;

	bool bad = false;
};
