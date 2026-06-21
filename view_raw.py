from PIL import Image
import numpy as np
import os

# Define the expected dimensions for the raw image data
width, height = 512, 512

# List of raw binary files expected to be generated from prior image processing steps
files_to_view = ["0_input.raw", "1_blurred.raw", "2_magnitude.raw", "3_direction.raw", "4_final_edges.raw"]

# Iterate through the target files to convert them to a standard image format
for file in files_to_view:
    # Verify file existence to prevent runtime errors during the read operation
    if os.path.exists(file):
        
        # Read the 8-bit unsigned integer data from the RAW file 
        # and reshape the 1D array into a 2D matrix matching the image dimensions
        data = np.fromfile(file, dtype=np.uint8).reshape((height, width))
        
        # Instantiate a PIL Image object from the NumPy array utilizing grayscale mode ('L')
        img = Image.fromarray(data, mode='L')
        
        # Construct the output filename by replacing the file extension
        out_name = file.replace(".raw", ".png")
        
        # Save the resulting image as a PNG file
        img.save(out_name)
        # Output a confirmation message to the standard output
        print(f"Saved {out_name}")
