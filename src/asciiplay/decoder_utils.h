#pragma once

#define readLE8(in, out) in.read(reinterpret_cast<char*>(&out), 1)
#define readLE16(in, out) in.read(reinterpret_cast<char*>(&out), 2)
#define readLE32(in, out) in.read(reinterpret_cast<char*>(&out), 4)
#define readLE64(in, out) in.read(reinterpret_cast<char*>(&out), 8)
#define readFloat(in, out) readLE32(in, out)