import sys
from PIL import Image

def png_to_bitmap(input_path, output_path, dimension):
    if dimension % 8 != 0:
        raise ValueError("Dimension must be a multiple of 8.")

    # Open the image and convert to RGB
    img = Image.open(input_path).convert('RGB')

    if img.size != (dimension, dimension):
        raise ValueError(f"Image must be {dimension}x{dimension} pixels.")

    bitmap = bytearray()
    current_byte = 0
    bit_index = 0

    for y in range(dimension):
        for x in range(dimension):
            r, g, b = img.getpixel((x, y))
            bit = 1 if (r, g, b) == (0, 0, 0) else 0

            current_byte |= (bit << bit_index)
            bit_index += 1

            if bit_index == 8:
                bitmap.append(current_byte)
                current_byte = 0
                bit_index = 0

    with open(output_path, 'wb') as f:
        f.write(bitmap)

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: python script.py <input.png> <output.bin> <dimension>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    try:
        dim = int(sys.argv[3])
    except ValueError:
        print("Error: <dimension> must be an integer.")
        sys.exit(1)

    try:
        png_to_bitmap(input_file, output_file, dim)
        print(f"Bitmap saved to {output_file}")
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
