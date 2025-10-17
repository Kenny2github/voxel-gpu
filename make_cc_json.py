import json
from os import getcwd
import sys
from glob import glob

cwd = getcwd()
data = [
    {
        'directory': cwd,
        'arguments': sys.argv[1:] + [filename, '-o', filename + '.o'],
        'file': filename,
    }
    for filename in glob('**/*.c', recursive=True)
]
print(json.dumps(data, indent='\t'))
