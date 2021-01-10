#pragma once
#include <istream>
#include <vector>
#include <cstdint>
#include <variant>

struct Position
{
	int x = 0;
	int y = 0;
};

struct Size
{
	int width = 0;
	int height = 0;
};

typedef uint8_t CharacterIndex;
typedef std::vector<char> CharacterSet;
typedef std::pair<CharacterIndex, Position> Pixel;
typedef std::vector<CharacterIndex> IFrame;
typedef std::vector<Pixel> PFrame;
typedef std::variant<IFrame, PFrame> Frame;
typedef std::vector<Frame> FrameList;

struct Audio
{
	enum Format {
		WAV,
		FLAC,
		MP3,
		OGG
	};

	Format format = Audio::WAV;
	char* data;
	uint32_t length = 0;
};

// This is also used for images
// (images are just videos with a single frame)
struct Video
{
	CharacterSet charset;
	float sx = 0;
	float sy = 0;
	float fps = 0;

	int width = 0;
	int height = 0;
	FrameList frames;
	Audio* audio = nullptr;

	bool loaded = false;
};