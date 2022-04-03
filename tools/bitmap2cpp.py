#
# Bitmap To C++
#

import sys
import os
import getopt
import png

HEXCHARS = "0123456789abcdef"

class Rect:
    def __init__(self, left, top, right, bottom):
        self.set(left, top, right, bottom)

    def set(self, left, top, right, bottom):
        self.left = left
        self.top = top
        self.right = right
        self.bottom = bottom
        self.width = right - left
        self.height = bottom - top

class Bitmap:

    def __init__(self, width, height, bits_per_pixel):
        self.set(width, height, bits_per_pixel, None)

    def __init__(self, width, height, bits_per_pixel, bitmap):
        self.set(width, height, bits_per_pixel, bitmap)

    def set(self, width, height, bits_per_pixel, bitmap):
        self.width_ = width
        self.height_ = height
        self.bits_per_pixel_ = bits_per_pixel

        self.bytes_per_line_ = int((width * self.bits_per_pixel_) / 8)
        self.size_ = height * self.bytes_per_line_
        self.bitmap_ = bytearray(self.size_)

        self.palette_size_ = 0
        if self.bits_per_pixel_ <= 8:
            self.palette_size_ = (1 << self.bits_per_pixel_)

        if bitmap:
            self.copy_data(bitmap)

    def copy_data(self, data):
        y = 0
        for bit_row in data:
            ofs = y * self.bytes_per_line_
            for bits in bit_row:
                self.bitmap_[ofs] = bits
                ofs += 1
            y += 1

    def set_palette_size(self, sz):
        self.palette_size_ = sz

    def write(self, filename):

        palette_data = None
        palette_data_size = 0
        if self.palette_size_ > 0:
            palette_data_size = self.palette_size_ * 4
            palette_data = bytearray(palette_data_size)
            for i in range(self.palette_size_):
                v = int((i * 255) / (self.palette_size_-1))
                Bitmap.putRGBA(palette_data, i*4, v, v, v, 255)

        header = [
            #OFS 0
            66, 77,   0, 0, 0, 0,   0, 0, 0, 0,   54, 0, 0, 0, # type, size, reserved, offbits
            #OFS 14
            40, 0, 0, 0,    0, 0, 0, 0,   0, 0, 0, 0,  # hdrsize, width, height
            #OFS 26
            1, 0,   24, 0,   0, 0, 0, 0, # planes, bitcount, compression
            #OFS 34
            0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,   # size, xpix/meter, ypix/meter
            #OFS 46
            0, 0, 0, 0,    0, 0, 0, 0   # clrused, clrimportant
        ]

        Bitmap.putInt32(header, 2, 54 + self.size_ + palette_data_size)
        Bitmap.putInt32(header, 18, self.width_)
        Bitmap.putInt32(header, 22, self.height_)
        Bitmap.putInt16(header, 28, self.bits_per_pixel_)

        if self.bits_per_pixel_ == 4 or self.bits_per_pixel_ == 8:
            Bitmap.putInt32(header, 46, self.palette_size_)

        f = open(filename, "wb")
        f.write(bytes(header))
        if palette_data:
            f.write(bytes(palette_data))
        f.write(bytes(self.bitmap_))
        f.close()

    def get_ofs(self, x, y):
        if x < 0 or y < 0 or x >= self.width_ or y >= self.height_: return -1
        return ((self.height_-y-1) * self.bytes_per_line_) + int((x * self.bits_per_pixel_)/8)

    def set_pixel(self, x, y, col):
        ofs = self.get_ofs(x, y)
        if -1 == ofs: return

        bpp = self.bits_per_pixel_
        if bpp <= 8:
            self.bitmap_[ofs] = col
        elif 16 == bpp:
            self.bitmap_[ofs+0] = col
            self.bitmap_[ofs+1] = col
        elif 24 == bpp:
            self.bitmap_[ofs+0] = col
            self.bitmap_[ofs+1] = col
            self.bitmap_[ofs+2] = col
        elif 32 == bpp:
            self.bitmap_[ofs+0] = col
            self.bitmap_[ofs+1] = col
            self.bitmap_[ofs+2] = col

    def putInt32(buf, ofs, value):
        buf[ofs+0] = value % 256
        buf[ofs+1] = (value >> 8) % 256
        buf[ofs+2] = (value >> 16) % 256
        buf[ofs+3] = (value >> 24) % 256
        return ofs+4

    def putRGBA(buf, ofs, r, g, b, a):
        # byte order: BGRA
        buf[ofs+0] = b
        buf[ofs+1] = g
        buf[ofs+2] = r
        buf[ofs+3] = a
        return ofs+4

    def putInt16(buf, ofs, value):
        buf[ofs+0] = value % 256
        buf[ofs+1] = (value >> 8) % 256
        return ofs+2


def colorToFloat(c):
    f = (float(c) + 0.5) / 255.0
    return min(1.0, f)

def brightness(r, g, b):
    return (r + g + b) / 3.0

def process(input_file, rect=None, index=None, flag_alpha=False, flag_special_color_filter=False):
    r = png.Reader(input_file)
    width, height, rows, info = r.read()
    bytes_per_pixel = info["planes"]
    has_alpha = info["alpha"]
    bits_per_pixel = bytes_per_pixel * info["bitdepth"]
    palette = None
    if "palette" in info:
        palette = info["palette"]

    bit_rows = []

    if not rect:
        rect = Rect(0, 0, width, height)

    y = 0
    for row in rows:

        if y < rect.top:
            y += 1
            continue
        elif y >= rect.bottom:
            break

        bit = 0
        accumulator = 0
        alpha_accumulator = 0
        bit_row = []

        if flag_alpha:
            alpha_row = []

        for x in range(rect.left, rect.right):

            ofs = x * bytes_per_pixel

            visible = False
            transparent = False

            if 24 == bits_per_pixel:
                r = colorToFloat(row[ofs+0])
                g = colorToFloat(row[ofs+1])
                b = colorToFloat(row[ofs+2])
                if not flag_special_color_filter:
                    visible = brightness(r,g,b) >= 1.0
                else:
                    visible = (g < 1.0 and b < 1.0 and r == 1.0)
            elif 32 == bits_per_pixel:
                r = colorToFloat(row[ofs+0])
                g = colorToFloat(row[ofs+1])
                b = colorToFloat(row[ofs+2])
                visible = brightness(r,g,b) >= 0.5
                if has_alpha:
                    a = row[ofs+3]
                    if a < 128:
                        transparent = True
            elif 8 == bits_per_pixel or 4 == bits_per_pixel or 2 == bits_per_pixel:
                if palette:
                    idx = row[ofs]
                    col = palette[idx]
                    r = col[0]
                    g = col[1]
                    b = col[2]
                    visible = brightness(r,g,b) >= 0.5
                    if len(col) > 3:
                        a = col[3]
                        if a < 128:
                            transparent = True
                else:
                    visible = row[ofs] != 0x0
            elif 1 == bits_per_pixel:
                visible = row[ofs] != 0x0

            if visible:
                accumulator += (1 << (7-bit))

            if flag_alpha and not transparent:
                alpha_accumulator += (1 << (7-bit))

            bit += 1

            if 8 == bit:
                bit = 0
                bit_row.append(accumulator)
                accumulator = 0
                if flag_alpha:
                    bit_row.append(alpha_accumulator)
                    alpha_accumulator = 0

        if 0 != bit:
            bit = 0
            bit_row.append(accumulator)
            accumulator = 0
            if flag_alpha:
                bit_row.append(alpha_accumulator)
                alpha_accumulator = 0

        bit_rows.append(bit_row)

        y += 1

    name = os.path.splitext(input_file)[0]

    s = to_string(name, rect.width, rect.height, bit_rows, flag_alpha, index)

    return s # , rect.width, rect.height, bit_rows

def format_bits(bits):
    return "0x" + HEXCHARS[int(bits/16)] + HEXCHARS[int(bits%16)]

def format_txt(txt, width):
    if len(txt) < width:
        txt += " " * (width - len(txt))
    return txt

def to_string(name, width, height, data, flag_alpha, index):

    lines = []

    appendix = f"_{index}" if index else ""

    lines.append("////////////////////////////////////////////////////////////////////////////////")
    lines.append(f"// Bitmap '{name}'")
    lines.append("////////////////////////////////////////////////////////////////////////////////")

    has_alpha = "true" if flag_alpha else "false"
    bytes_per_line = len(data[0])
    bitmap_size = len(data) * bytes_per_line

    bitmap_symbol_name = f"{name}_bitmap_pixels{appendix}"

    lines.append(f"static const uint8_t {bitmap_symbol_name}[] = {{")

    y = 0
    for bit_row in data:
        s = "  "
        element = 0
        for bits in bit_row:
            s += format_bits(bits)
            if y < height-1 or element < len(bit_row)-1:
                s += ", "
            element += 1
        lines.append(s)
        y += 1

    lines.append("};\n")

    output_width = len(bitmap_symbol_name) + 4

    lines.append(f"const graphics::bitmap_t {name}_bitmap{appendix} = {{")
    lines.append(format_txt(f"    {width},", output_width) + "  // width")
    lines.append(format_txt(f"    {height},", output_width) + "  // height")
    lines.append(format_txt(f"    {has_alpha},", output_width) + "  // true if alpha channel")
    lines.append(format_txt(f"    {bytes_per_line},", output_width) + "  // bytes per line")
    lines.append(format_txt(f"    {bitmap_size},", output_width) + "  // bitmap size")
    lines.append(format_txt(f"    {bitmap_symbol_name}", output_width) + "  // pixel data")
    lines.append("};")
    lines.append("\n")

    s = "\n".join(lines)

    return s

def save(filename, content):
    lines = []
    lines.append("////////////////////////////////////////////////////////////////////////////////")
    lines.append("// Bitmap data")
    lines.append("// @generated")
    lines.append("// clang-format off")
    lines.append("////////////////////////////////////////////////////////////////////////////////")
    lines.append("\n")
    header = "\n".join(lines)

    with open(filename, "w") as text_file:
        text_file.write(header)
        text_file.write(content)

def usage():
    print("Usage  : bitmap2cpp INPUT OUTPUT")
    print("")
    print("INPUT  : Input image file. Currently, only the PNG file format is supported")
    print("OUTPUT : C++ source file to be generated")

def main():

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hao:", ["alpha", "help", "output="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    output = None
    alpha = False
    for o, a in opts:
        if o in ("-a", "--alpha"):
            alpha = True
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-o", "--output"):
            output = a

    s = ""
    for arg in args:
        s += process(arg, None, None, alpha)

    if output:
        save(output, s)
    else:
        print(s)

if __name__ == "__main__":
    main()
