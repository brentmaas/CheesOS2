#!/usr/bin/env python3

from PIL import Image
import sys
import numpy as np
import os
import argparse

GLYPH_ROWS = 32

HEADER_TEMPLATE = '''\
#ifndef _CHEESOS2_RES_FONT_H
#define _CHEESOS2_RES_FONT_H

#include "driver/vga/text.h"

{entries}

#endif
'''

HEADER_ENTRY_TEMPLATE = 'extern const vga_font {};'

SOURCE_TEMPLATE = '''\
{includes}

{entries}
'''

SOURCE_ENTRY_TEMPLATE = 'const vga_font {} = {};'

FONT_BASE_PATH = 'res/font'

def generate_font(path):
    font_name = os.path.splitext(os.path.basename(path))[0]
    data = np.array(Image.open(path).convert("L"))
    h, w = data.shape

    if w % 16 != 0 or h % 16 != 0:
        raise ValueError(f'Dimensions of font "{font_name}" must be multiples of 16, for a total of 256 characters')

    char_w, char_h = w // 16, h // 16
    if char_w != 8:
        raise ValueError(f'Character width of font "{font_name}" must be 8 (currently {char_w})')

    data = np.swapaxes(data.reshape((16, char_h, 16, char_w)), 1, 2)
    data = 1 - np.minimum(data, 1)
    data = np.packbits(data, axis = -1).reshape((256, char_h))
    data = np.pad(data, ((0, 0), (0, GLYPH_ROWS - char_h)), 'constant', constant_values = 0)

    return (font_name, data)

def generate_fonts(fonts):
    def format_glyph(glyph):
        return '{' + ', '.join(f'0x{row:02X}' for row in glyph) + '}'

    def format_font(font):
        return '{\n    ' + ',\n    '.join(map(format_glyph, font)) + '\n}'

    header_entries = []
    source_entries = []

    for path in fonts:
        font_name, font_data = generate_font(path)
        font_expr = format_font(font_data)

        name = f'FONT_{font_name.upper()}'
        header_entries.append(HEADER_ENTRY_TEMPLATE.format(name))
        source_entries.append(SOURCE_ENTRY_TEMPLATE.format(name, font_expr))

    return header_entries, source_entries

parser = argparse.ArgumentParser(description = 'Create header/source C files from font PNG\'s')
parser.add_argument('fonts', metavar = '<path>', nargs = '+', help = 'Add font path')
parser.add_argument('-C', metavar = '<path>', required = True, help = 'Output location of generated source file')
parser.add_argument('-H', metavar = '<path>', required = True, help = 'Output location of generated header file')
parser.add_argument('-I', metavar = '<include>', action = 'append', help = 'Add source include directive')

args = parser.parse_args()

header_entries, source_entries = generate_fonts(args.fonts)
header = HEADER_TEMPLATE.format(
    entries = '\n'.join(header_entries)
)

source = SOURCE_TEMPLATE.format(
    includes = '\n'.join(f'#include "{include}"' for include in args.I),
    entries = '\n\n'.join(source_entries)
)

with open(args.C, 'w') as f:
    f.write(source)

with open(args.H, 'w') as f:
    f.write(header)
