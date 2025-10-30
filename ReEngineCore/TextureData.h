#pragma once


struct TextureData {
	int width = 0;
	int height = 0;
	int channels = 0;
	std::vector<unsigned char> pixels;
	std::string type;
	std::string path;

	TextureData() {};

	TextureData(int w, int h, int c, unsigned char* d)
		: width(w), height(h), channels(c) {
		pixels.assign(d, d + (w * h * c));
	}
};