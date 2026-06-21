from PIL import Image
import numpy as np

# 1. Load the source image, convert it to 8-bit grayscale ('L' mode), 
# and resize the dimensions to exactly 512x512 pixels.
img = Image.open("test.jpg").convert('L').resize((512, 512))

# 2. Convert the PIL Image object into a NumPy array, 
# explicitly specifying 8-bit unsigned integers (uint8) to represent pixel intensities.
img_data = np.array(img, dtype=np.uint8)

# 3. Serialize and export the raw array data directly to a binary file 
# without any metadata or headers.
img_data.tofile("input.raw")

# Output a confirmation message to the standard output to indicate success.
print("Image successfully converted to input.raw (512x512)")
