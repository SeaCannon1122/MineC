from PIL import Image
import struct
import os
import argparse

def process_8x8_block(block):
    max_width = 0
    layout = []

    for row in range(8):
        byte = 0
        row_width = 0
        for col in range(8):
            pixel = block.getpixel((col, row))
            if pixel == 0:  # Assuming black is 0
                byte |= (1 << col)  # Set the bit corresponding to the column
                row_width = col + 1
        max_width = max(max_width, row_width)
        layout.append(byte)

    # Convert layout to bytes
    layout_bytes = struct.pack('8B', *layout)

    if max_width == 0:
        max_width = 3

    return max_width, layout_bytes

def png_to_pixelfont(input_png):
    img = Image.open(input_png).convert('L')  # Convert image to grayscale
    width, height = img.size

    # Ensure the image dimensions are correct
    assert width == 8*256 and height == 8, "Image dimensions should be 8x(8*256) pixels."

    # Create output filename with .pixelfont extension
    base_name = os.path.splitext(input_png)[0]
    output_pixelfont = base_name + '.pixelfont'

    with open(output_pixelfont, 'wb') as f:
        for i in range(256):
            block = img.crop((i*8, 0, (i+1)*8, 8))  # Crop the 8x8 block
            max_width, layout_bytes = process_8x8_block(block)

            # Pack the data: first 8 bytes for max width, then 8 bytes for layout
            f.write(struct.pack('Q', max_width))  # Write max_width as a 64-bit integer
            f.write(layout_bytes)  # Write the 8 bytes of layout

    print(f"Output saved to: {output_pixelfont}")

def main():
    parser = argparse.ArgumentParser(description="Convert an 8x(8x256) PNG file to a .pixelfont file.")
    parser.add_argument('input_png', type=str, help='Input PNG file name')
    args = parser.parse_args()

    png_to_pixelfont(args.input_png)

if __name__ == "__main__":
    main()
