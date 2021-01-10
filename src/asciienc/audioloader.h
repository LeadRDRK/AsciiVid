#pragma once
#include <string>
#include <types.h>

namespace AudioLoader
{
    bool loadAudio(std::string &filename, Audio* audio);
}