from PIL import Image
import struct
import os
import argparse

def process_16x16_block(block):
    max_width = 0
    layout = []

    for row in range(16):
        byte1 = 0
        byte2 = 0
        row_width = 0
        for col in range(16):
            pixel = block.getpixel((col, row))
            if pixel == 0:  # Assuming black is 0
                if col < 8:
                    byte1 |= (1 << col)  # Set the bit corresponding to the column in byte1
                else:
                    byte2 |= (1 << (col - 8))  # Set the bit corresponding to the column in byte2
                row_width = col + 1
        max_width = max(max_width, row_width)
        layout.append(byte1)
        layout.append(byte2)

    # Convert layout to bytes (2 bytes per row, 16 rows total)
    layout_bytes = struct.pack('32B', *layout)

    if max_width == 0:
        max_width = 6

    return max_width, layout_bytes

def png_to_pixelfont(input_png):
    img = Image.open(input_png).convert('L')  # Convert image to grayscale
    width, height = img.size

    # Ensure the image dimensions are correct
    assert width == 16*256 and height == 16, "Image dimensions should be 16x(16*256) pixels."

    # Create output filename with .pixelfont extension
    base_name = os.path.splitext(input_png)[0]
    output_pixelfont = base_name + '.pixelfont'

    with open(output_pixelfont, 'wb') as f:
        for i in range(256):
            block = img.crop((i*16, 0, (i+1)*16, 16))  # Crop the 16x16 block
            max_width, layout_bytes = process_16x16_block(block)

            # Pack the data: first 8 bytes for max width, then 32 bytes for layout (since it's 16x16)
            f.write(struct.pack('Q', max_width))  # Write max_width as a 64-bit integer
            f.write(layout_bytes)  # Write the 32 bytes of layout

    print(f"Output saved to: {output_pixelfont}")

def main():
    parser = argparse.ArgumentParser(description="Convert a 16x(16x256) PNG file to a .pixelfont file.")
    parser.add_argument('input_png', type=str, help='Input PNG file name')
    args = parser.parse_args()

    png_to_pixelfont(args.input_png)

if __name__ == "__main__":
    main()
