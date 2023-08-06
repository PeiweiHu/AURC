# A script to extract macros and enum

## Requires

tree_sitter>=0.2

argparse>=1.1

## Usage

```bash
python extract_macros_enum.py -f path/to/files -o path/to/output/
```

## Output
A C Programming Language file corresponds to two output files
e.g. test.c -> path/to/output/test_macros.txt and path/to/output/test_enum.txt
