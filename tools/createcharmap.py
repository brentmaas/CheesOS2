#!/usr/bin/env python3

from PIL import Image
import sys
import numpy as np
import os

if not os.path.isfile(sys.argv[1]):
    print(f"Char map {sys.argv[1]} does not exist!")
    exit(1)

#if not os.path.isfile(sys.argv[3]):
#    print(f"Header file {sys.argv[3]} does not exist!")
#    exit(1)

data = np.array(Image.open(sys.argv[1]).convert("L"))

"""with open(sys.argv[2], "w") as f:
    f.write(f"#include \"{sys.argv[3]}\"\n\n")
    f.write("const vga_font font {\n")
    for i, j in np.ndindex(16, 16):
        f.write("    {")
        for k in range(16):
            f.write(("," * (k > 0)) + "0b" + ("".join((data[16*i+k,8*j:8*j+8] == 0).astype(np.int32).astype(np.str_))))
        f.write("}" + ("," * (not(i == 15 and j == 15))) + "\n")
    f.write("};")"""

print(data.shape)
with open(sys.argv[2], 'w') as f:
    
    # TODO
    
    f.write('{')
    for pixel in range(0, 256):
        f.write('{')
        vijf = 0
        for j, i in np.ndindex(16, 8):
            x = (pixel % 16) * 8 + i
            y = (pixel // 16) * 16 + j
            
            if i == 0:
                vijf = 0
                if j > 0:
                    f.write(',')
            
            vijf |= (~data[y][x] // 255) << i
            
            if i == 7:
                f.write(hex(vijf))
        f.write('},\n')
    f.write('}')