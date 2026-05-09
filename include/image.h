#ifndef IMAGE_H
#define IMAGE_H
#include <vector>
#include <string>

struct Image {
    int width;
    int height;
    std::vector<unsigned char> data;
};

Image readPGM(const std::string& filename);
bool writePGM(const std::string& filename, const Image& img);
#endif
