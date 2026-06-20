% RAW Image Converter
% Converts an input image to a 512x512 grayscale RAW file compatible
% with the C++ image processing pipeline.

clear;
clc;

input_image_path = 'my_photo.jpg';
output_raw_path  = 'input.raw';

try
% Load input image
img = imread(input_image_path);

```
% Convert RGB images to grayscale
if size(img, 3) == 3
    img_gray = rgb2gray(img);
else
    img_gray = img;
end

% Resize to the format expected by the processing pipeline
img_resized = imresize(img_gray, [512, 512]);

% Create output RAW file
fileID = fopen(output_raw_path, 'wb');
if fileID == -1
    error('Failed to create output RAW file.');
end

% Convert MATLAB column-major layout to C/C++ row-major layout
fwrite(fileID, img_resized', 'uint8');

fclose(fileID);

fprintf('Success: %s generated (512x512 grayscale RAW)\n', output_raw_path);
```

catch ME
fprintf('Error: %s\n', ME.message);
end
