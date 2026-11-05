from os import listdir
from PIL import Image

final_output = [0] * 50_000_000;
final_output_c = 0

for x in sorted(listdir(".")):
    print(f"{x}...")
    if x.startswith("frame"):
        img = Image.open(x)
        last = 0
        counter = 0
        for y in range(120):
            for x_ in range(120):
                p = img.getpixel((x_, y))
                norm = 1 if p[0] else 0
                if last == norm:
                    counter += 1
                    if counter == 0b111_1111_1111_1111:
                        final = (last << 15) | counter
                        final_output[final_output_c] = final
                        final_output_c += 1
                        counter = 0
                else:
                    if counter != 0:
                        final = (last << 15) | counter
                        final_output[final_output_c] = final
                        final_output_c += 1
                    last = norm
                    counter = 1
        if counter:
            final = (last << 15) | counter
            final_output[final_output_c] = final
            final_output_c += 1


with open('data', 'wb') as e:
    for x in final_output[:final_output_c]:
        e.write(x.to_bytes(2, 'little'))
