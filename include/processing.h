#ifndef PROCESSING_H
#define PROCESSING_H
#include "image.h"

Image applyGaussianBlur(const Image& input);


void applySobelRVV(const Image& input, Image& magnitude, Image& direction);
void applySobel(const Image& input, Image& mag, Image& dir, bool use_l2 = true);
Image applyCannyPostProcessing(const Image& magnitude, const Image& direction);

// Double Thresholding and Hysteresis functions
Image applyDoubleThresholding(const Image& nms_img, unsigned char low_thresh, unsigned char high_thresh);
Image applyHysteresis(const Image& thresh_img);
#endif
