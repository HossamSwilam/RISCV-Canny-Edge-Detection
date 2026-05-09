#ifndef PROCESSING_H
#define PROCESSING_H
#include "image.h"

Image applyGaussianBlur(const Image& input);
void applySobel(const Image& input, Image& magnitude, Image& direction);
Image applyCannyPostProcessing(const Image& magnitude, const Image& direction);
#endif
