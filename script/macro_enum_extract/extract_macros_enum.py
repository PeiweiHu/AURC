import os
import utils
import argparse
from tree_sitter import Language, Parser

def walk_all_nodes(cursor, move, types: list, nodes: list):
    node = cursor.node
    type = node.type
    if move == 0:
        if len(types) == 0 or type in types:
            nodes.append(node)
        if cursor.goto_first_child():
            walk_all_nodes(cursor, 0, types, nodes)
        elif cursor.goto_next_sibling():
            walk_all_nodes(cursor, 1, types, nodes)
        elif cursor.goto_parent():
            walk_all_nodes(cursor, 2, types, nodes)
    elif move == 1:
        if len(types) == 0 or type in types:
            nodes.append(node)
        if cursor.goto_first_child():
            walk_all_nodes(cursor, 0, types, nodes)
        elif cursor.goto_next_sibling():
            walk_all_nodes(cursor, 1, types, nodes)
        elif cursor.goto_parent():
            walk_all_nodes(cursor, 2, types, nodes)
    elif move == 2:
        if cursor.goto_next_sibling():
            walk_all_nodes(cursor, 1, types, nodes)
        elif cursor.goto_parent():
            walk_all_nodes(cursor, 2, types, nodes)

def get_content(node, src) -> str:
    start = node.start_byte
    end = node.end_byte
    return src[start:end]

def extract_macros(tree, src, output):
    cursor = tree.walk()
    nodes = []
    walk_all_nodes(cursor, 0, ['preproc_def', 'preproc_function_def'], nodes)
    with open(output, 'w', newline='', encoding='utf-8') as f:
        for node in nodes:
            id_node = node.child_by_field_name("name")
            arg_node = node.child_by_field_name("value")
            id = get_content(id_node, src).strip()
            arg = get_content(arg_node, src).strip()
            if node.type == 'preproc_function_def':
                arg = arg.split('(')[0].strip()
            f.write(id + ' ' + arg + '\n')

def extract_enum(tree, src, output):
    cursor = tree.walk()
    nodes = []
    walk_all_nodes(cursor, 0, ['enum_specifier'], nodes)
    with open(output, 'w', newline='', encoding='utf-8') as f:
        for node in nodes:
            num = -1
            body_node = node.child_by_field_name("body")
            for child_node in body_node.children:
                if child_node.type == 'enumerator':
                    id_node = child_node.child_by_field_name("name")
                    val_node = child_node.child_by_field_name("value")
                    id = get_content(id_node, src).strip()
                    if val_node is not None:
                        val = get_content(val_node, src).strip()
                        num = int(val)
                    else:
                        num += 1

                    f.write(id + ' ' + str(num) + '\n')

if __name__ == '__main__':
    """
    Input path of files.
    """
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument('-f', "--path", help="Path of files. dir or file", required=True)
    parser.add_argument('-o', "--out", help="Path of output files. dir", required=True)

    args = parser.parse_args()
    path = args.path
    out_path = args.out

    if (not os.path.isdir(out_path)):
        print("Output path is not a directory or the directory does not exist!")
        exit(0)

    files = utils.get_files(path)
    if len(files) == 0:
        print("No C Programming Language files to analyze!")
        exit(0)

    C_LANGUAGE = Language('./build/parse-c.so', 'c')
    parser = Parser()
    parser.set_language(C_LANGUAGE)

    for f in files:
        src = utils.read_src(f)
        tree = parser.parse(bytes(src.encode('utf-8')))
        macros_output = os.path.join(out_path, os.path.splitext(os.path.basename(f))[0] + '_macros.txt')
        enum_output = os.path.join(out_path, os.path.splitext(os.path.basename(f))[0] + '_enum.txt')
        extract_macros(tree, src, macros_output)
        extract_enum(tree, src, enum_output)


