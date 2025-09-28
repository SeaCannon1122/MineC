import os

def file_to_c_array(filename, varname):
    with open(filename, "rb") as f:
        data = f.read()

    # Format into hex bytes
    hex_bytes = [f"0x{b:02x}" for b in data]
    
    # Break into lines of max 12 bytes for readability
    lines = []
    for i in range(0, len(hex_bytes), 12):
        lines.append(", ".join(hex_bytes[i:i+12]))

    array_content = ",\n    ".join(lines)

    c_array = (
        f"const uint8_t {varname}[] = {{\n"
        f"    {array_content}\n"
        f"}};\n\n"
    )
    return c_array


def generate_header(files, outname=".serialized_files/data_files.h"):
    os.makedirs(os.path.dirname(outname), exist_ok=True)
    with open(outname, "w") as out:
        out.write("#pragma once\n\n")
        out.write("#include <stdint.h>\n\n")
        for filepath in files:
            varname = os.path.basename(filepath).replace(".", "_").replace("-", "_")
            out.write(file_to_c_array(filepath, varname))
            out.write("\n")
    print(f"Header file '{outname}' generated successfully.")


if __name__ == "__main__":
#opengl
    generate_header(["shaders/pixelchar_gl.vert", "shaders/pixelchar_gl.frag"], ".shader_headers/pixelchar_impl_opengl_shaders.h")
    
#vulkan 
    os.system("glslc.exe shaders/pixelchar_vk.vert -o shaders/pixelchar_vk.vert.spv")
    os.system("glslc.exe shaders/pixelchar_vk.frag -o shaders/pixelchar_vk.frag.spv")
    
    generate_header(["shaders/pixelchar_vk.vert.spv", "shaders/pixelchar_vk.frag.spv"], ".shader_headers/pixelchar_impl_vulkan_shaders.h")
