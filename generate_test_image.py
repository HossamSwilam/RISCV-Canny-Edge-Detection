import struct

def write_pgm(filename, width, height, pixels):
    with open(filename, 'wb') as f:
        f.write(f"P5\n{width} {height}\n255\n".encode())
        f.write(bytes(pixels))

width, height = 128, 128
pixels = [0] * (width * height)


for y in range(40, 90):
    for x in range(40, 90):
        pixels[y * width + x] = 255


for x in range(10, 118):
    pixels[20 * width + x] = 255


for y in range(10, 118):
    pixels[y * width + 108] = 255

write_pgm("tests/input.pgm", width, height, pixels)
print("Done! tests/input.pgm created (128x128)")
