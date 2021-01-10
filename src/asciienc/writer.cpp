#include "writer.h"
#include "types.h"
#include "writer_utils.h"
#include <array>
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

// TODO: split these writers into their own separate files

// ASV writer
#define ASV_VERSION 100
static char ASV_SIG[4] = {'A', 'S', 'V', 0x06};
static char ASV_CHARSET_SIG[4] = {0x6e, 0x33, 0x00, 0x1a};
static char ASV_FRAMES_SIG[4] = {0x1e, 0x1d, 0x01, 0x09};
static char ASV_AUDIO_SIG[4] = {0x7f, -0x01, 0x14, 0x12};

bool writeASV(Video &video, std::ostream &out)
{
	bool hasAudio = video.audio != nullptr;

	std::cout << "Writing ASV file...\n" << std::endl;
	/* header */
	std::cout << "== Header..." << std::endl;
	out.write(ASV_SIG, 4); // signature
	writeLE16(out, ASV_VERSION); // version
	writeLE16(out, video.width); // width
	writeLE16(out, video.height); // height
	writeFloat(out, video.fps); // fps
	writeLE32(out, video.frames.size()); // total frames count
	writeLE8(out, hasAudio); // has audio

	/* charset */
	std::cout << "== Charset..." << std::endl;
	out.write(ASV_CHARSET_SIG, 4); // signature
	writeLE8(out, video.charset.size()); // size
	for (int i = 0; i < video.charset.size(); i++)
	{
		char c = video.charset[i];
		writeLE8(out, c); // character
	}

	/* frames start signature */
	out.write(ASV_FRAMES_SIG, 4);

	/* frames */
	std::cout << "== Frames..." << std::endl;
	for (int i = 0; i < video.frames.size(); i++)
	{
		auto frame = video.frames[i];
		int type = frame.index();

		writeLE8(out, type); // frame type

		switch (type)
		{
			case 0: // I-frame (list of character indices)
			{
				IFrame fr = std::get<IFrame>(frame);
				for (int n = 0; n < fr.size(); n++)
				{
					CharacterIndex chr = fr[n];
					writeLE8(out, chr); // character index
				}
				break;
			}
			case 1: // P-frame (list of character indices, with
			        // specific position to place them)
			{
				PFrame fr = std::get<PFrame>(frame);
				writeLE32(out, fr.size()); // number of entries
				for (int n = 0; n < fr.size(); n++)
				{
					Pixel pixel = fr[n];
					writeLE8(out, pixel.first); // character index
					Position &pos = pixel.second;
					writeLE16(out, pos.x); // x position
					writeLE16(out, pos.y); // y position
				}
				break;
			}
		}
	}

	/* audio */
	if (hasAudio) {
		std::cout << "== Audio..." << std::endl;
		Audio* audio = video.audio;

		/* header */
		out.write(ASV_AUDIO_SIG, 4); // signature
		writeLE8(out, audio->format); // format
		writeLE32(out, audio->length); // length
		/* data */
		out.write(audio->data, audio->length);
	}

	std::cout << "== Write successful!\n" << std::endl;
	return true;
}

bool writeTXT(Video &video, std::ostream &out)
{
	std::cout << "Writing TXT file...\n" << std::endl;

	std::cout << "== Frames..." << std::endl;
	if (video.frames.size() == 1)
	{
		// single frame export
		IFrame fr = std::get<IFrame>(video.frames[0]);
		for (int y = 0; y < video.height; y++)
		{
			for (int x = 0; x < video.width; x++)
			{
				int i = (y * video.width) + x;
				CharacterIndex chr = fr[i];
				out << video.charset[chr];
			}
			out << "\n";
		}
	}
	else
	{
		// multiple frames export
		std::vector<char> frameBuffer;
		for (int n = 0; n < video.frames.size(); n++)
		{
			auto frame = video.frames[n];
			int type = frame.index();
			out << "Frame " << n << "\n";

			// write the frame to the frame buffer
			switch (type)
			{
				case 0: // I-frame
				{
					IFrame fr = std::get<IFrame>(frame);
					frameBuffer.resize(fr.size());
					for (int i = 0; i < fr.size(); i++)
					{
						CharacterIndex chr = fr[i];
						frameBuffer[i] = video.charset[chr];
					}
					break;
				}
				case 1: // P-frame
				{
					PFrame fr = std::get<PFrame>(frame);
					for (int i = 0; i < fr.size(); i++)
					{
						Pixel pixel = fr[i];
						Position &pos = pixel.second;
						int pxNum = (pos.y * video.width) + pos.x;
						frameBuffer[pxNum] = video.charset[pixel.first];
					}
					break;
				}
			}

			// write the frame buffer to the stream
			for (int y = 0; y < video.height; y++)
			{
				for (int x = 0; x < video.width; x++)
				{
					int i = (y * video.width) + x;
					out << frameBuffer[i];
				}
				out << "\n";
			}

			out << "\n";
		}
	}

	std::cout << "== Write successful!\n" << std::endl;
	return true;
}

bool Writer::WriteStream(Video &video, std::ostream &out, std::string &outFormat)
{
	auto iter = formatMap.find(outFormat);

	if (iter == formatMap.end()) {
		std::cout << "FATAL: Unknown format " << outFormat << std::endl;
		return false;
	}

	switch (iter->second)
	{
	case FORMAT_ASV:
		return writeASV(video, out);
	case FORMAT_TXT:
		return writeTXT(video, out);
	default:
		return false;
	}
}

bool Writer::WriteFile(Video &video, std::string &filename, std::string &outFormat)
{
	fs::path path = filename;
	if (outFormat.empty()) {
		outFormat = path.extension().string();
	}

	std::cout << "File to write: " << path << std::endl;

	std::ofstream out(filename, std::ios::trunc);
	bool ret = WriteStream(video, out, outFormat);
	out.close();
	return ret;
}
