#
# Blender Mesh Export
#

import bpy
import os

output_buffer = ""

def writeLn(txt):
    global output_buffer
    print(txt)
    output_buffer += txt
    output_buffer += '\n'

def write(txt):
    global output_buffer
    print(txt, end='')
    output_buffer += txt

def export_current_object():

    global output_buffer

    obdata = bpy.context.object.data

    writeLn('////////////////////////////////////////////////////////////////////////////////')
    writeLn('// Mesh data')
    writeLn('// @generated')
    writeLn('// clang-format off')
    writeLn('////////////////////////////////////////////////////////////////////////////////\n')

    writeLn('static const std::vector<graphics::Vertex> vertices {')
    for v in obdata.vertices:
        write(f'    {{ {v.co.x}, {v.co.y}, {v.co.z} }}')
        writeLn(',') if v != obdata.vertices[-1] else writeLn('')
    writeLn('};\n')

    writeLn('static const std::vector<graphics::Face> faces {')
    for f in obdata.polygons:
        if len(f.vertices) == 3:
            write(f'    {{ {f.vertices[0]}, {f.vertices[1]}, {f.vertices[2]} }}')
        elif len(f.vertices) == 4:
            write(f'    {{ {f.vertices[0]}, {f.vertices[1]}, {f.vertices[2]}, {f.vertices[3]} }}')
        else:
            continue
        writeLn(',') if f != obdata.polygons[-1] else writeLn('')
    writeLn('};')

    document_path = os.path.expanduser('~/Documents')
    out_file = document_path + '/mesh.inc'

    f = open(out_file, 'w')
    f.writelines(output_buffer)
    f.close()

    print(f'\nwritten mesh data to {out_file}')

export_current_object()
