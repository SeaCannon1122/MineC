import os
import sys

LINE_LEN = 16

def cerialize_hash(name, table_len):
    h = 5381
    for b in name.encode('utf-8'):
        h = ((h << 5) + h + b) & 0xFFFFFFFF
    return h % table_len

def to_c_symbol(path):
    return path.replace('\\', '___').replace('/', '___').replace('.', '__')

def make_c_array(data, line_len=LINE_LEN):
    lines = []
    for i in range(0, len(data), line_len):
        chunk = data[i:i+line_len]
        line = ', '.join(f'0x{b:02x}' for b in chunk)
        lines.append(line)
    return ',\n    '.join(lines)

def process_files(cerialize_dir):
    file_data = []
    for root, _, files in os.walk(cerialize_dir):
        for file in files:
            abs_path = os.path.join(root, file)
            rel_path = os.path.relpath(abs_path, cerialize_dir)
            with open(abs_path, 'rb') as f:
                data = f.read()
            if not data:
                continue
            
            print(f"[Cerialize] Processing file: {rel_path}")

            name_sym = to_c_symbol(rel_path) + "__name"
            data_sym = to_c_symbol(rel_path) + "__data"
            c_path = rel_path.replace("\\", "/")
            file_data.append({
                'rel_path': rel_path,
                'c_path': c_path,
                'data': data,
                'name_sym': name_sym,
                'data_sym': data_sym
            })
    return file_data

def write_header_files(file_data, out_dir, base_dir):
    for f in file_data:
        header_path = os.path.join(out_dir, base_dir, os.path.dirname(f['rel_path']))
        os.makedirs(header_path, exist_ok=True)
        file_name = os.path.basename(f['rel_path']) + '.h'
        header_file = os.path.join(header_path, file_name)
        with open(header_file, 'w') as hf:
            hf.write('#pragma once\n')
            hf.write('#include <stdint.h>\n\n')
            hf.write(f'static const uint8_t {f["data_sym"]}[] = {{\n    {make_c_array(f["data"])}\n}};\n\n')
            hf.write(f'static const uint8_t {f["name_sym"]}[] = "{f["c_path"]}";\n')

def write_fs_map(file_data, out_dir, base_dir, table_size=64):
    os.makedirs(out_dir, exist_ok=True)
    h_path = os.path.join(out_dir, f"{base_dir}.h")
    c_path = os.path.join(out_dir, f"{base_dir}.c")

    with open(h_path, 'w') as hf:
        hf.write('#pragma once\n')
        hf.write('#include <stdint.h>\n')
        hf.write('#include "cerialize.h"\n\n')
        hf.write(f'extern const struct cerialized_file_system* const cerialized_{base_dir}_file_system;\n')

    with open(c_path, 'w') as cf:
        cf.write(f'#include "{base_dir}.h"\n\n')
        included = set()
        for f in file_data:
            inc_path = os.path.join(base_dir, f['rel_path'] + '.h').replace('\\', '/')
            if inc_path not in included:
                cf.write(f'#include "{inc_path}"\n')
                included.add(inc_path)

        cf.write('\n')

        cf.write('//uint32_t cerialized_hash(uint8_t* str, uint32_t table_len) {\n')
        cf.write('//    uint32_t hash = 5381;\n')
        cf.write('//    while (*str) hash = ((hash << 5) + hash) + *str++;\n')
        cf.write('//    return hash % table_len;\n')
        cf.write('//}\n\n')

        entry_defs = []
        buckets = [[] for _ in range(table_size)]

        for i, f in enumerate(file_data):
            sym = f"entry_{i}"
            h = cerialize_hash(f['c_path'], table_size)
            buckets[h].append(sym)

        # Define entries with proper chaining
        for i, f in enumerate(file_data):
            h = cerialize_hash(f['c_path'], table_size)
            sym = f"entry_{i}"
            bucket = buckets[h]
            idx_in_bucket = bucket.index(sym)
            next_ptr = f"(struct cerialized_file_system_entry*) &{bucket[idx_in_bucket-1]}" if idx_in_bucket > 0 else "NULL"
            entry_defs.append(
                f'static const struct cerialized_file_system_entry {sym} = {{ .next = {next_ptr}, .name = {f["name_sym"]}, .data = {f["data_sym"]}, .size = sizeof({f["data_sym"]}) }};'
            )

        cf.write('\n'.join(entry_defs) + '\n\n')

        cf.write(f'static const struct cerialized_file_system_entry* {base_dir}_entries[{table_size}] = {{\n')
        for i, bucket in enumerate(buckets):
            if bucket:
                cf.write(f'    [{i}] = &{bucket[-1]},\n')
            else:
                cf.write(f'    [{i}] = NULL,\n')
        cf.write('};\n\n')

        cf.write(f'const struct cerialized_file_system* const cerialized_{base_dir}_file_system = &(const struct cerialized_file_system) {{\n')
        cf.write(f'    .table_length = {table_size},\n')
        cf.write(f'    .entries = {base_dir}_entries\n')
        cf.write('};\n')

def main():
    
    cerialize_dir = "resources"

    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path_to_cerialize_dir> -> assuming 'resources' as default folder")
    else:
        cerialize_dir = sys.argv[1]
        
    base_dir = os.path.basename(os.path.abspath(cerialize_dir))
    out_dir = ".cerialized_src"

    file_data = process_files(cerialize_dir)
    if not file_data:
        print(f"No files to cerialize in '{cerialize_dir}'")
        sys.exit(1)

    write_header_files(file_data, out_dir, base_dir)
    write_fs_map(file_data, out_dir, base_dir, table_size=max(64, len(file_data)*2))
    print(f"Cerialization completed. Output written to {out_dir}")

if __name__ == "__main__":
    main()
