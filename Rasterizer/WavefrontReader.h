
#ifndef __WAVEFRONT_READER_H__
#define __WAVEFRONT_READER_H__

#include <vector>
#include "geometry.h"

class WavefrontReader {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
public:
	WavefrontReader(const char* filename);
	~WavefrontReader();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
};

#endif //__WAVEFRONT_READER_H__

