import os

def get_files(path):
    files = []
    for _r, _d, _fs in os.walk(path):
        for _f in _fs:
            if any([_f.endswith(ext) for ext in ['.c', '.h']]):
                   files.append(os.path.join(_r, _f))

    return files

def read_src(src_path):
        with open(src_path, 'r', encoding='utf-8') as f:
            src = f.read()
        return src
