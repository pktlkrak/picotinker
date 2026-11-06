from os import listdir
from PIL import Image
from lzma import compress

final_output = []

for x in sorted(listdir(".")):
    print(f"{x}...")
    if x.startswith("frame"):
        img = Image.open(x)
        last = 0
        counter = 0
        frame = 7200 * [0]
        px = 0
        for y in range(240):
            for x_ in range(240):
                p = img.getpixel((x_, y))
                if p[0] > 120:
                    p = 1
                else:
                    p = 0
                px = p | (px << 1)
                if x_ and (x_ % 8) == 7:
                    frame[(x_ + y * 240) // 8] = px
                    px = 0
        final_output.append(compress(bytes(frame)))


with open('data', 'wb') as e:
    for x in final_output:
        e.write(len(x).to_bytes(2, 'little'))
        e.write(x)
