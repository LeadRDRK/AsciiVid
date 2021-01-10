#include "encoder.h"
#include <png.hpp>
using png::rgb_pixel;

#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <cmath>
namespace fs = std::filesystem;
using system_clock = std::chrono::system_clock;

#define PRINT_RET(source, ret) std::cout << \
	"[" << source << "] Return code: " << ret << std::endl;

inline uint8_t getLuma(uint8_t r, uint8_t g, uint8_t b) {
	return 0.2126*r + 0.7152*g + 0.0722*b;
}

inline CharacterIndex getCharacterIndex(uint8_t &luma, CharacterSet &charset)
{
	return std::round(luma * ((float)(charset.size()-1) / 255.f));
}

bool frameShouldBeIFrame(IFrame& frame, IFrame &prevFrame)
{
	int pixelsChanged = 0;
	for (int i = 0; i < frame.size(); i++)
	{
		if (frame[i] != prevFrame[i])
			pixelsChanged++;
	}
	return ((float)pixelsChanged / (float)frame.size()) >= 0.5;
}

void toPFrame(IFrame& frame, IFrame &prevFrame, int width, PFrame &output)
{
	for (int i = 0; i < frame.size(); i++)
	{
		if (frame[i] != prevFrame[i])
		{
			Position pos;
			pos.y = i/width;
			pos.x = i - (pos.y * width);

			output.push_back({frame[i], pos});
		}
	}
}

bool encodePng(std::string inputFile, CharacterSet &charset, float sx, float sy,
			   IFrame &output, int &width, int &height)
{
	png::image<rgb_pixel> image(inputFile, png::require_color_space<rgb_pixel>());
	width = image.get_width() * sx;
	height = image.get_height() * sy;

	for (int y = 0; y < height; y++)
	{
		auto row = image.get_row(y / sy);
		for (int x = 0; x < width; x++)
		{
			rgb_pixel px = row[x / sx];
			uint8_t luma = getLuma(px.red, px.green, px.blue);
			
			CharacterIndex index = getCharacterIndex(luma, charset);
			output.push_back(index);
		}
	}

	return true;
}

bool encodeFramesInDir(std::string directory, std::string inFormat, Video &video)
{
	if (!video.fps) {
		std::cout << "Error: FPS must be specified" << std::endl;
		return false;
	}

	int i = 1;
	int iFrCount = 0,
	    pFrCount = 0,
	    fps = 0;
	std::chrono::time_point start = system_clock::now();
	fs::path frameFile = directory + "/" + std::to_string(i) + inFormat;
	IFrame prevFrame;

	while (fs::exists(frameFile)) {
		std::chrono::duration<float> timeElapsed = system_clock::now() - start;
		if (timeElapsed.count() >= 1) {
			std::cout << "Frame: " << i << " | "
			          << "FPS: " << fps << std::endl;
			start = system_clock::now();
			fps = 0;
		}

		if (!fs::is_regular_file(frameFile)) {
			std::cout << "FATAL: " << frameFile << " is not a regular file" << std::endl;
			return false;
		}

		// encode the frame
		IFrame frame;
		bool ret = encodePng(frameFile.string(), video.charset, video.sx, video.sy,
		                     frame, video.width, video.height);
		if (!ret) return false;

		// push the frame to the output
		// convert the i-frame to a p-frame if needed
		if (prevFrame.empty() || frameShouldBeIFrame(frame, prevFrame))
		{
			video.frames.push_back(frame);
			iFrCount++;
		}
		else {
			PFrame pFrame;
			toPFrame(frame, prevFrame, video.width, pFrame);
			video.frames.push_back(pFrame);
			pFrCount++;
		}

		// get ready for the next loop
		prevFrame = frame;
		fps++;
		i++;
		frameFile = directory + "/" + std::to_string(i) + inFormat;
	}

	std::cout << "\n";

	if (video.frames.size() == 0) {
		std::cout << "WARNING: No frame was encoded\n" << std::endl;
	}

	std::cout << "== Encoding successful!\n"
	          << "== Total frames: " << i - 1 << "\n"
	          << "==     I-Frames: " << iFrCount << "\n"
	          << "==     P-Frames: " << pFrCount << "\n"
	<< std::endl;

	return true;
}

bool Encoder::encodeFrames(std::string &inputFile, std::string &inFormat, Video &video)
{
	if (!fs::exists(inputFile)) {
		std::cout << "FATAL: Input file does not exist" << std::endl;
		return false;
	}

	fs::path path = inputFile;
	std::cout << "Input: " << path << std::endl;
	if (inFormat.empty()) {
		inFormat = path.extension().string();
	}

	if (fs::is_directory(path)) {
		if (inFormat.empty()) {
			std::cout << "FATAL: The input format must specified using --ifmt" << std::endl;
			return false;
		}
		return encodeFramesInDir(inputFile, inFormat, video);
	}
	else if (fs::is_regular_file(path)) {
		if (inFormat == ".png") {
			// encode a single frame
			IFrame frame;
			bool ret = encodePng(inputFile, video.charset, video.sx, video.sy,
			                     frame, video.width, video.height);
			if (!ret) return ret;

			video.frames.push_back(frame);
		} else {
			// TODO: encode from video files
			std::cout << "FATAL: File unsupported" << std::endl;
			return false;
		}
	}
	else {
		std::cout << "FATAL: Input file/device not supported" << std::endl;
		return false;
	}

	return true;
}