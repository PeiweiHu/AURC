import sys
import os


def extract_bc(dir_path):
    if not os.path.exists(dir_path):
        print('invalid path...')
        return None
    bc_files = []
    for _r, _d, _fs in os.walk(dir_path):
        for _f in _fs:
            _f_path = os.path.join(_r, _f)
            if _f_path.endswith('.bc'):
                bc_files.append(_f_path)
    return bc_files


def extract_bc_to(proj_path, dst_path):
    if not os.path.exists(proj_path) or not os.path.exists(dst_path):
        print('invalid path...')
        return
    if not os.path.isdir(dst_path):
        print('the dst is not a directory...')
        return
    # copy the bitcode files in proj_path to dst_path
    bc_files = extract_bc(proj_path)
    cp_cmd = 'cp {0} {1}'
    for _b in bc_files:
        fname = os.path.basename(_b)
        os.system(cp_cmd.format(_b, os.path.join(dst_path, fname.lstrip('.'))))


def print_bc(dir_path):
    bc_files = extract_bc(dir_path)
    for _b in bc_files:
        print(_b + ' ')


if __name__ == '__main__':

    help = """
        python extract_bc.py -p [project path] -> print the paths of all bitcode files in [project path]
        python extract_bc.py -c [from] [to] -> copy all bitcode files in [from] to [to]
    """
    if len(sys.argv) == 3 and sys.argv[1] == '-p':
        proj_path = sys.argv[2]
        print_bc(proj_path)
    elif len(sys.argv) == 4 and sys.argv[1] == '-c':
        proj_path = sys.argv[2]
        dst_path = sys.argv[3]
        extract_bc_to(proj_path, dst_path)
    else:
        print(help)
