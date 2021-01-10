#include "audioloader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
namespace fs = std::filesystem;

static std::unordered_map<std::string, Audio::Format> formatMap {
    {".wav",  Audio::WAV},
    {".flac", Audio::FLAC},
    {".mp3",  Audio::MP3},
    {".ogg",  Audio::OGG}
};

bool loadFile(std::string &filename, char* &data, uint32_t &length)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.bad()) {
        std::cout << "FATAL: Failed to load " << filename << std::endl;
        return false;
    }

    length = file.tellg();
    if (length == 0) {
        std::cout << "WARNING: Loaded file size is 0" << std::endl;
    }
    data = new char[length];

    file.seekg(0, std::ios::beg);
    file.read(data, length);
    file.close();

    return true;
}

bool AudioLoader::loadAudio(std::string &filename, Audio *audio)
{
    fs::path path = filename;
    if (!fs::exists(path)) {
        std::cout << "FATAL: " << path << " does not exist" << std::endl;
        return false;
    }
    if (!fs::is_regular_file(path)) {
        std::cout << "FATAL: " << path << " is not a regular file" << std::endl;
        return false;
    }

    std::string ext = path.extension().string();
    auto iter = formatMap.find(ext);
    if (iter == formatMap.end())
    {
        std::cout << "FATAL: Audio format unsupported" << std::endl;
        return false;
    }
    
    audio->format = iter->second;
    std::cout << "Reading audio file..." << std::endl;
    return loadFile(filename, audio->data, audio->length);
}