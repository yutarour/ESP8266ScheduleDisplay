import os
from PIL import Image
import numpy as np

def rgb_to_hex(r, g, b):
    #8bit rgb888
    #return '0x{:01x}{:01x}{:01x}'.format(r, g, b)
    return ("0x%0.4X" % ((int(r / 255 * 31) << 11) | (int(g / 255 * 63) << 5) | (int(b / 255 * 31))))

files_to_convert = os.listdir("images")
img_arrs = []
for file in files_to_convert:
    img = Image.open("images/"+file)
    img_arr = np.array(img)
    img_arrs.append(img_arr)
    


with open("out.txt",'w') as file:
    file.write(f"const uint16_t PROGMEM  ")
    for i,img_arr in enumerate(img_arrs):
        file.write(f"img{i}[] = {{ {len(img_arrs[0][0])},{len(img_arrs[0])},")
        for row in img_arr:
            for pixel in row:
                file.write(rgb_to_hex(pixel[0], pixel[1], pixel[2])+",")
            file.write("\n")
        #if not last one add ,
        if i != len(img_arrs)-1:
            file.write("},\n")
        else:
            file.write("};")
    
