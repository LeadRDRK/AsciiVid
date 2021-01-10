#pragma once
#include <types.h>
#include <string>

namespace Writer {
	bool WriteStream(Video &video, std::ostream &out, std::string &outFormat);
	bool WriteFile(Video &video, std::string &filename, std::string &outFormat);
}
