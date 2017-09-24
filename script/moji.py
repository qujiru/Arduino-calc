'''
This script generates data files for calculator.
'''
from PIL import Image, ImageDraw, ImageFont
import sys

import imglcdnize

def gen_image(char, size = (12, 16), fontname = "Minecraft.ttf", fontsize = 16):
    img = Image.new('1', size, 'white')
    draw = ImageDraw.Draw(img)
    font = ImageFont.truetype(fontname, fontsize)
    draw.text((0, 0), char, font=font, fill='#000')
    return img

def img_proc(img):
    img = img.quantize(4).convert('1')
    return img

def lcdnize(img, imgx, imgy):
    pages = []
    pagenum = int(imgy / 8);
    for p in range(0, pagenum):    #page
        page = []
        for x in range(0, imgx):        #col
            binary = '0b'
            for y in range(p * 8, p * 8 + 8)[::-1]:       #pixel
                tmp = 1 if (img.getpixel((x*2, y*2)) == 0) else 0
                binary = binary + str(tmp)
            page.append(hex(int(binary, 2)))
        pages.append(page)
    return pages

def main():
    for c in range(10):
        img = gen_image(str(c))
        (imgx, imgy) = img.size
        dat =  lcdnize(img, int(imgx/2), int(imgy/2))
        imglcdnize.save(dat, 'chars/'+str(c) + '.lpi')

if __name__ == '__main__':
    main()
