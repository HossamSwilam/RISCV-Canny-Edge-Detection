from PIL import Image
import numpy as np
import os

width, height = 512, 512
files_to_view = ["0_input.raw", "1_blurred.raw", "2_magnitude.raw", "3_direction.raw"]

for file in files_to_view:
    if os.path.exists(file):
        # قراءة ملف الـ RAW
        data = np.fromfile(file, dtype=np.uint8).reshape((height, width))
        # تحويله لصورة
        img = Image.fromarray(data, mode='L')
        # حفظه كـ PNG
        out_name = file.replace(".raw", ".png")
        img.save(out_name)
        print(f"Saved {out_name}")
