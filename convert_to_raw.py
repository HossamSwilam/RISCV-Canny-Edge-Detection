from PIL import Image
import numpy as np

# 1. افتح الصورة وحولها لرمادي (Grayscale) واعملها Resize لـ 512x512
img = Image.open("test.jpg").convert('L').resize((512, 512))

# 2. حولها لمصفوفة أرقام (8-bit)
img_data = np.array(img, dtype=np.uint8)

# 3. احفظها كملف RAW
img_data.tofile("input.raw")
print("Image successfully converted to input.raw (512x512)")
