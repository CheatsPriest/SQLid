import os

def count_lines(directory):
    total_lines = 0
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.h', '.cpp', '.hpp')):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8') as f:
                        lines = len(f.readlines())
                        total_lines += lines
                        print(f"{filepath}: {lines}")
                except UnicodeDecodeError:
                    # Пробуем другие кодировки
                    try:
                        with open(filepath, 'r', encoding='cp1251') as f:
                            lines = len(f.readlines())
                            total_lines += lines
                            print(f"{filepath}: {lines} (cp1251)")
                    except:
                        with open(filepath, 'r', encoding='latin-1') as f:
                            lines = len(f.readlines())
                            total_lines += lines
                            print(f"{filepath}: {lines} (latin-1)")
    return total_lines

print(f"Total: {count_lines('src')} lines")
