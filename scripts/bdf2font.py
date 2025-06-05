import sys
import struct

def parse_bdf(filename):
    print(f"Opening file: {filename}")
    with open(filename, "r") as f:
        lines = [line.rstrip() for line in f.readlines()]
    print(f"Read {len(lines)} lines from file")

    glyphs = []
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith("STARTCHAR"):
            print(f"Found STARTCHAR at line {i}")
            codepoint = None
            bitmap = []
            width = 16  # Default width
            while not lines[i].strip().startswith("ENDCHAR"):
                parts = lines[i].strip().split()
                if not parts:  # Skip empty lines
                    i += 1
                    continue
                if parts[0] == "ENCODING":
                    codepoint = int(parts[1])
                    print(f"Found ENCODING: {codepoint}")
                elif parts[0] == "BBX":
                    w, h = map(int, parts[1:3])
                    print(f"Found BBX: {w}x{h}")
                    width = w
                    if h != 16:
                        print(f"Skipping unsupported height: {h}")
                        break  # Skip unsupported height
                elif parts[0] == "BITMAP":
                    print("Found BITMAP section")
                    i += 1
                    for _ in range(16):
                        hex_line = lines[i].strip()
                        # Convert hex string to integer, handling both formats
                        if hex_line.startswith("0x"):
                            value = int(hex_line, 16)
                        else:
                            # Pad with zeros to make it 4 characters (16 bits)
                            hex_line = hex_line.zfill(4)
                            value = int(hex_line, 16)
                        # If width is 8, we need to pad to 16 bits
                        if width == 8:
                            value = value << 8
                        bitmap.append(value)
                        i += 1
                    continue
                i += 1
            if codepoint is not None and len(bitmap) == 16:
                print(f"Adding glyph for codepoint {codepoint}")
                glyphs.append((codepoint, bitmap))
            else:
                print(f"Skipping incomplete glyph: codepoint={codepoint}, bitmap length={len(bitmap)}")
        i += 1
    return glyphs

def write_font_file(glyphs, output_file):
    with open(output_file, "wb") as f:
        # Write header
        f.write(struct.pack("<I H B B I", 0x55464E54, len(glyphs), 16, 16, 0))
        for codepoint, bitmap in glyphs:
            f.write(struct.pack("<I", codepoint))
            # Convert each 16-bit value to two bytes
            for value in bitmap:
                f.write(struct.pack("<H", value))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python bdf2font.py unifont.bdf unifont.font")
        sys.exit(1)

    glyphs = parse_bdf(sys.argv[1])
    write_font_file(glyphs, sys.argv[2])
    print(f"Converted {len(glyphs)} glyphs to {sys.argv[2]}")
