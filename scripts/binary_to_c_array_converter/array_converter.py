import sys
import os

def convert_to_c_byte_array(input_file, output_file, array_name):
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
        
        with open(output_file, 'w') as out:
            out.write(f"unsigned char {array_name}[] = {{\n")
            
            # Write bytes in hex format, 12 bytes per line for readability
            for i, byte in enumerate(data):
                out.write(f"  0x{byte:02x},")
                if (i + 1) % 12 == 0:
                    out.write("\n")
            out.write("\n};\n\n")
            
            # Write the length of the array
            out.write(f"unsigned int {array_name}_len = {len(data)};\n")
        
        print(f"Successfully wrote C byte array to {output_file}")

    except Exception as e:
        print(f"Error: {e}")

def main():
    if len(sys.argv) != 4:
        print("Usage: python convert_spv_to_c.py <input_file.spv> <output_file.h> <array_name>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    array_name = sys.argv[3]

    if not os.path.isfile(input_file):
        print(f"Error: The file '{input_file}' does not exist.")
        sys.exit(1)

    convert_to_c_byte_array(input_file, output_file, array_name)

if __name__ == "__main__":
    main()
