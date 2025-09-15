import yaml
import struct
import sys
from pathlib import Path
from PIL import Image
import os

# Helper function: pad a bytearray to next multiple of 32
def pad32(data: bytearray) -> bytearray:
    padding = (32 - (len(data) % 32)) % 32
    return data + bytearray(padding)

# Convert bitmap (2D array of 0/1) to packed uint32 row-major
def bitmap_to_uint32(bitmap, resolution):
    flat_bits = []
    for row in bitmap:
        flat_bits.extend(row)
    # Pad flat_bits to multiple of 32
    if len(flat_bits) % 32 != 0:
        flat_bits += [0] * (32 - len(flat_bits) % 32)
    uint32_array = []
    for i in range(0, len(flat_bits), 32):
        val = 0
        for bit in flat_bits[i:i+32]:
            val = (val << 1) | bit
        uint32_array.append(val)
    return uint32_array

# Convert a single PNG to multiple bitmaps
def png_to_bitmaps(png_path: Path, resolution: int, verbose=False):
    if verbose:
        print(f"Loading PNG {png_path}...")
    img = Image.open(png_path).convert("RGB")
    width, height = img.size
    if width % resolution != 0 or height % resolution != 0:
        raise ValueError(f"PNG {png_path} dimensions not divisible by resolution {resolution}")
    bitmaps = []

    tiles_x = width // resolution
    tiles_y = height // resolution
    total_tiles = tiles_x * tiles_y

    if verbose:
        print(f"PNG {png_path} contains {total_tiles} bitmaps ({tiles_x}x{tiles_y} grid)")

    for ty in range(tiles_y):
        for tx in range(tiles_x):
            tile = img.crop((tx*resolution, ty*resolution, (tx+1)*resolution, (ty+1)*resolution))
            bitmap = []
            for y in range(resolution):
                row = []
                for x in range(resolution):
                    r, g, b = tile.getpixel((x, y))
                    row.append(1 if (r, g, b) == (0, 0, 0) else 0)
                bitmap.append(row)
            bitmaps.append(bitmap)
    return bitmaps

def main():
    if len(sys.argv) < 2:
        print("Usage: python generate_pixelfont.py <config_file.yaml> [--verbose]")
        return

    config_path = Path(sys.argv[1])
    verbose = "--verbose" in sys.argv or "-v" in sys.argv

    if not config_path.exists():
        print(f"Config file {config_path} not found.")
        return

    # Change working directory to config file's folder
    os.chdir(config_path.parent)
    if verbose:
        print(f"Changed working directory to {config_path.parent}")

    # Load YAML config
    if verbose:
        print(f"Loading config {config_path}...")
    with open(config_path.name, "r") as f:
        config = yaml.safe_load(f)

    name = config["name"].encode("ascii")[:32]
    name = name + b"\x00" * (32 - len(name))  # null-terminated, padded to 32
    resolution = config["resolution"]
    default_bitmap_index = config["default_bitmap_index"]
    mappings_files = [Path(p) for p in config["mappings_files"]]
    metadata_files = [Path(p) for p in config["bitmap_metadata_files"]]
    png_files = [Path(p) for p in config["bitmap_image_files"]]
    output_file = Path(config["output_file"])

    # --- Process mappings ---
    if verbose:
        print("Processing mappings files...")
    mappings = []
    for mp_file in mappings_files:
        if verbose:
            print(f"Reading mappings from {mp_file}...")
        with open(mp_file, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line:
                    mappings.append(int(line))
    mapping_count = len(mappings)
    if verbose:
        print(f"Total mappings: {mapping_count}")
    mappings_bytes = bytearray()
    for m in mappings:
        mappings_bytes += struct.pack("<I", m)
    mappings_bytes = pad32(mappings_bytes)
    if verbose:
        print(f"Mappings section size (padded): {len(mappings_bytes)} bytes")

    # --- Process metadata ---
    if verbose:
        print("Processing metadata files...")
    metadata_list = []
    for meta_file in metadata_files:
        
        print(meta_file, os.path.exists(meta_file), os.path.getsize(meta_file))
        if verbose:
            print(f"Reading metadata from {meta_file}...")
        with open(meta_file, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line:
                    width, thickness = map(int, line.split())
                    metadata_list.append( (width, thickness) )
    bitmap_count = len(metadata_list)
    if verbose:
        print(f"Total bitmaps (from metadata): {bitmap_count}")
    metadata_bytes = bytearray()
    for width, thickness in metadata_list:
        metadata_bytes += struct.pack("<II", width, thickness)
    metadata_bytes = pad32(metadata_bytes)
    if verbose:
        print(f"Metadata section size (padded): {len(metadata_bytes)} bytes")

    # --- Verify mappings ---
    if verbose:
        print("Verifying mappings...")
    for i, m in enumerate(mappings):
        if m < 0 or m >= bitmap_count:
            raise ValueError(f"Mapping value at index {i} = {m} is out of range [0, {bitmap_count - 1}]")
    if verbose:
        print("Mappings verification passed.")

    # --- Process bitmap PNGs ---
    if verbose:
        print("Processing bitmap PNG files...")
    all_bitmaps = []
    for png_file in png_files:
        bitmaps = png_to_bitmaps(png_file, resolution, verbose)
        all_bitmaps.extend(bitmaps)
    if len(all_bitmaps) != bitmap_count:
        raise ValueError(f"Total bitmaps ({len(all_bitmaps)}) != metadata entries ({bitmap_count})")
    if verbose:
        print(f"Total bitmaps extracted: {len(all_bitmaps)}")

    bitmap_bytes = bytearray()
    for i, bitmap in enumerate(all_bitmaps):
        if verbose:
            print(f"Packing bitmap {i+1}/{len(all_bitmaps)}...")
        uint32_array = bitmap_to_uint32(bitmap, resolution)
        for val in uint32_array:
            bitmap_bytes += struct.pack("<I", val)
    bitmap_bytes = pad32(bitmap_bytes)
    if verbose:
        print(f"Bitmap data section size (padded): {len(bitmap_bytes)} bytes")

    # --- Build header ---
    if verbose:
        print("Building header...")
    header_struct = bytearray()
    # placeholder header section size
    header_struct += struct.pack("<IIII", 0, len(mappings_bytes), len(metadata_bytes), len(bitmap_bytes))
    header_struct += name
    header_struct += struct.pack("<IIII", mapping_count, default_bitmap_index, resolution, bitmap_count)
    header_bytes = pad32(header_struct)
    # fill correct header_section_size
    header_section_size = len(header_bytes)
    header_bytes[0:4] = struct.pack("<I", header_section_size)
    if verbose:
        print(f"Header section size (padded): {header_section_size} bytes")

    # --- Write final file ---
    if verbose:
        print(f"Writing final pixelfont to {output_file}...")
    with open(output_file, "wb") as f:
        f.write(header_bytes)
        f.write(mappings_bytes)
        f.write(metadata_bytes)
        f.write(bitmap_bytes)

    if verbose:
        print("Write complete.")
    print(f"Successfully wrote {output_file}")

if __name__ == "__main__":
    main()
