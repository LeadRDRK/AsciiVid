#pragma once
#include <fstream>

// mmm give me that sugar
inline void writeLE8(std::ostream &p, uint8_t v)
{
	p << v;
}

inline void writeLE16(std::ostream &p, uint16_t v)
{
	p << (uint8_t)v
	  << (uint8_t)(v >> 8);
}

inline void writeLE32(std::ostream &p, uint32_t v)
{
    p << (uint8_t)v
	  << (uint8_t)(v >> 8)
	  << (uint8_t)(v >> 16)
	  << (uint8_t)(v >> 24);
}

inline void writeLE64(std::ostream &p, uint64_t v)
{
	writeLE32(p, (uint32_t)v);
	writeLE32(p, (uint32_t)(v >> 32));
}

union FloatBytes {
	float f; 
	char  b[sizeof(float)];
};

inline void writeFloat(std::ostream &p, float v)
{
	FloatBytes fb = {v};
	for(int i = 0; i < sizeof(float); i++)
        p << fb.b[i];
}