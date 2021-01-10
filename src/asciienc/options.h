#pragma once
#include <types.h>
#include <string>

struct Options {
	Options(int &argc, char** &argv);
	bool help = false;
	std::string input;
	std::string audio;
	std::string output;
	CharacterSet charset;
	float fps = 0;
	float sx = 1;
	float sy = 0.5;
	
	std::string ifmt;
	std::string ofmt;
	bool listFormats = false;

	bool bad = false;
};
