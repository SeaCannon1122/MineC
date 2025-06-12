#!/usr/bin/env python3
import sys
from pathlib import Path

CERIALIZE_ROOT = ".cerialized_src"

class ResourceNode:
    def __init__(self, name):
        self.name = name
        self.children = {}
        self.is_file = False
        self.symbol = None

def sanitize_symbol(name):
    result = ''.join(c if c.isalnum() else '_' for c in name)
    if result and result[0].isdigit():
        result = '_' + result
    return result

def serialize_file_to_header(src_path, header_path, symbol_base):
    data = src_path.read_bytes()
    if len(data) == 0:
        return  # Skip empty files

    header_path.parent.mkdir(parents=True, exist_ok=True)

    with open(header_path, 'w') as f:
        f.write('#pragma once\n#include <stdint.h>\n#include <stddef.h>\n\n')
        f.write(f'static const uint8_t {symbol_base}__data[] = {{\n')
        for i in range(0, len(data), 12):
            chunk = data[i:i+12]
            f.write('    ' + ', '.join(f'0x{b:02x}' for b in chunk) + ',\n')
        f.write('};\n')

def add_path_to_tree(root, path_parts, symbol_name):
    node = root
    for part in path_parts[:-1]:
        if part not in node.children:
            node.children[part] = ResourceNode(part)
        node = node.children[part]
    leaf_name = path_parts[-1]
    if leaf_name not in node.children:
        leaf = ResourceNode(leaf_name)
        leaf.is_file = True
        leaf.symbol = symbol_name
        node.children[leaf_name] = leaf

def write_struct_fields(f, node, indent=1):
    ind = '    ' * indent
    for child_name in sorted(node.children):
        child = node.children[child_name]
        if child.is_file:
            field_name = sanitize_symbol(child.name.replace('.', '__'))
            f.write(f"{ind}struct cerialized_file {field_name};\n")
        else:
            f.write(f"{ind}struct {{\n")
            write_struct_fields(f, child, indent + 1)
            f.write(f"{ind}}} {child.name};\n")

def write_header_struct_file(resource_base_name, root, header_path):
    with open(header_path, 'w') as f:
        f.write("#pragma once\n#include <stdint.h>\n#include <stddef.h>\n\n")
        f.write("struct cerialized_file {\n    const uint8_t *data;\n    size_t size;\n};\n\n")
        struct_name = f"cerialized_{resource_base_name}"
        f.write(f"struct {struct_name} {{\n")
        write_struct_fields(f, root)
        f.write("};\n\n")
        f.write(f"extern struct {struct_name} {resource_base_name};\n")

def write_c_file(resource_base_name, root, c_path, header_dir):
    with open(c_path, 'w') as f:
        f.write(f'#include "{resource_base_name}.h"\n')

        def write_includes_rec(node, prefix_path):
            for child_name in sorted(node.children):
                child = node.children[child_name]
                if child.is_file:
                    rel_path = prefix_path / child.name
                    header_filename = rel_path.name + ".h"
                    include_path = f"{resource_base_name}/" + "/".join(rel_path.parts[:-1]) + "/" + header_filename
                    include_path = include_path.replace("//", "/")
                    f.write(f'#include "{include_path}"\n')
                else:
                    write_includes_rec(child, prefix_path / child_name)

        write_includes_rec(root, Path())

        struct_name = f"cerialized_{resource_base_name}"
        f.write(f"\nstruct {struct_name} {resource_base_name} = {{\n")

        def write_inits(node, indent=1):
            ind = "    " * indent
            for child_name in sorted(node.children):
                child = node.children[child_name]
                if child.is_file:
                    field_name = sanitize_symbol(child.name.replace('.', '__'))
                    f.write(f'{ind}.{field_name} = {{ .data = {child.symbol}__data, .size = sizeof({child.symbol}__data) }},\n')
                else:
                    f.write(f'{ind}.{child.name} = {{\n')
                    write_inits(child, indent + 1)
                    f.write(f'{ind}}},\n')
        write_inits(root)
        f.write("};\n")

def main(resource_dir):
    resource_dir = Path(resource_dir).resolve()
    if not resource_dir.is_dir():
        print(f"Error: {resource_dir} is not a directory.")
        sys.exit(1)

    resource_base_name = resource_dir.name
    output_dir = Path(CERIALIZE_ROOT)
    headers_output_dir = output_dir / resource_base_name
    headers_output_dir.mkdir(parents=True, exist_ok=True)

    root = ResourceNode(resource_base_name)

    for path in resource_dir.rglob("*"):
        if path.is_file():
            if path.stat().st_size == 0:
                print(f"Skipping empty file: {path}")
                continue

            rel_path = path.relative_to(resource_dir)
            symbol_name = sanitize_symbol(
                "__".join([resource_base_name] + list(rel_path.parts)).replace('.', '__')
            )
            header_path = headers_output_dir.joinpath(*rel_path.parts[:-1], rel_path.name + ".h")
            print(f"Cerializing: {path} -> {header_path}")
            serialize_file_to_header(path, header_path, symbol_name)
            add_path_to_tree(root, list(rel_path.parts), symbol_name)

    header_path = output_dir / f"{resource_base_name}.h"
    c_path = output_dir / f"{resource_base_name}.c"

    write_header_struct_file(resource_base_name, root, header_path)
    write_c_file(resource_base_name, root, c_path, headers_output_dir)

    print("Cerialization complete.")
    print(f"Header written to: {header_path}")
    print(f"Source written to: {c_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: Cerialize_resources.py <resource_dir>")
        sys.exit(1)
    main(sys.argv[1])
