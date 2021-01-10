#include "decoder.h"
#include "decoder_utils.h"
#include "types.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>
namespace fs = std::filesystem;

#define FORMAT_ASV 1
#define FORMAT_TXT 2

static std::unordered_map<std::string, int> formatMap {
	{".asv", FORMAT_ASV},
	{".txt", FORMAT_TXT}
};

#define ASV_VERSION 100
static uint32_t ASV_SIG = 0x06565341;
static uint32_t ASV_CHARSET_SIG = 0x1a00336e;
static uint32_t ASV_FRAMES_SIG = 0x09011d1e;
static uint32_t ASV_AUDIO_SIG = 0x1214ff7f;

inline bool verifySignature(std::ifstream &file, uint32_t &sig)
{
	uint32_t fileSig = 0;
	readLE32(file, fileSig);
	return fileSig == sig;
}

bool decodeASV(std::string &filename, Video &video)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cout << "FATAL: Failed to open " << filename << " for reading" << std::endl;
		return false;
	}

	// check the signature
	if (!verifySignature(file, ASV_SIG))
	{
		std::cout << "Error: Invalid file signature" << std::endl;
		return false;
	}

	// read the version
	uint16_t version = 0;
	readLE16(file, version);
	if (version > ASV_VERSION)
	{
		std::cout << "Error: This program is outdated, please upgrade" << std::endl;
		return false;
	}

	// read the other header entries
	readLE16(file, video.width);
	readLE16(file, video.height);
	readFloat(file, video.fps);
	uint32_t frameCount = 0;
	readLE32(file, frameCount);
	bool hasAudio = 0;
	readLE8(file, hasAudio);

	// check the charset signature
	if (!verifySignature(file, ASV_CHARSET_SIG))
	{
		std::cout << "Error: Invalid charset signature" << std::endl;
		return false;
	}

	// read the charset
	uint8_t charCount = 0;
	readLE8(file, charCount);
	for (int i = 0; i < charCount; i++)
	{
		char c = 0;
		readLE8(file, c);
		video.charset.push_back(c);
	}

	// check the frames signature
	if (!verifySignature(file, ASV_FRAMES_SIG))
	{
		std::cout << "Error: Invalid frames signature" << std::endl;
		return false;
	}

	// read the frames
	int pixelCount = video.width * video.height;
	int framesRead = 0;
	while (framesRead < frameCount)
	{
		uint8_t type = 0;
		readLE8(file, type);

		switch (type)
		{
		case 0: // I-frame
		{
			IFrame frame;
			for (int i = 0; i < pixelCount; i++) {
				CharacterIndex chr = 0;
				readLE8(file, chr);
				frame.push_back(chr);
			}
			video.frames.push_back(frame);
			break;
		}
		case 1: // P-frame
		{
			PFrame frame;
			uint32_t entryCount = 0;
			readLE32(file, entryCount);
			for (int i = 0; i < entryCount; i++) {
				Pixel pixel;
				readLE8(file, pixel.first);
				readLE16(file, pixel.second.x);
				readLE16(file, pixel.second.y);
				frame.push_back(pixel);
			}
			video.frames.push_back(frame);
			break;
		}
		default:
			std::cout << "Error: Invalid frame type " << unsigned(type) << std::endl;
			return false;
		}

		framesRead++;
	}

	if (hasAudio)
	{
		Audio* audio = new Audio;
		// verify the audio signature
		if (!verifySignature(file, ASV_AUDIO_SIG))
		{
			std::cout << "Error: Invalid audio signature" << std::endl;
			return false;
		}

		// read the header
		readLE8(file, audio->format);
		readLE32(file, audio->length);

		// read the data
		audio->data = new char[audio->length];
		file.read(audio->data, audio->length);
		if (file.gcount() < audio->length)
			std::cout << "WARNING: Audio data appears to be incomplete" << std::endl;

		video.audio = audio;
	}

	video.loaded = true;
	return true;
}

bool decodeTXT(std::string &filename, Video &video)
{
	return true;
}

bool Decoder::decodeFile(std::string &filename, Video &video)
{
	fs::path path = filename;
	if (!fs::exists(path)) {
		std::cout << "Error: Input file does not exist" << std::endl;
		return false;
	}
	if (!fs::is_regular_file(path)) {
		std::cout << "Error: Input is not a regular file" << std::endl;
		return false;
	}

	std::string ext = path.extension().string();
	auto iter = formatMap.find(ext);
	
	if (iter == formatMap.end()) {
		std::cout << "FATAL: Format not supported" << std::endl;
		return false;
	}

	switch (iter->second)
	{
	case FORMAT_ASV:
		return decodeASV(filename, video);
	case FORMAT_TXT:
		return decodeTXT(filename, video);
	default:
		return false;
	}
}