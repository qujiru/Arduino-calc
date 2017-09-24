'''
This script converts the image into the series of data for displaying on the LCD.
'''
import numpy as np
from PIL import Image, ImageFilter, ImageChops, ImageOps
import sys

flag_ino_data = False 
flag_senga = False

def readoption(opt):
    for c in opt:
        if (c == 'i'):
            print("write .ino data.")
            global flag_ino_data
            flag_ino_data = True
        elif (c == 's'):
            print("convert image to 'senbyou'")
            global flag_senga
            flag_senga = True
        else:
            if (c != '-'):
                print("Invalid option: ", c)
                print(" -i  : write .ino data")
                print(" -s  : convert image to 'senbyou'")
                quit()

def shirokuro(img):
    mimg = None
    if flag_senga:
        senga  = img.convert("L")
        senga2 = senga.filter(ImageFilter.MaxFilter(5))
        simg = ImageChops.difference(senga, senga2)
        simg = ImageOps.invert(simg)
        simg.thumbnail((128,48), Image.LANCZOS)
        mimg = simg.convert("1")
    else:
        mimg = img.copy().quantize(64)
        mimg.thumbnail((128,48), Image.LANCZOS)
        mimg = mimg.convert("L").convert('1')
    return mimg

def lcdnize(img, imgx, imgy):
    pages = []
    pagenum = int(imgy / 8);
    for p in range(0, pagenum):    #page
        page = []
        for x in range(0, imgx):        #col
            binary = '0b'
            for y in range(p * 8, p * 8 + 8)[::-1]:       #pixel
                tmp = 1 if (img.getpixel((x, y)) == 0) else 0
                binary = binary + str(tmp)
            page.append(hex(int(binary, 2)))
        pages.append(page)
    return pages

def save(pages, filename, imgx = 128):
    f= open(filename, "w")
    if flag_ino_data:
        f.write("/* paste this file to your .ino file.*/\n")
        f.write("//====== data start ======\n")
    cnt = 0
    for page in pages:
        if flag_ino_data:
            f.write("uint8_t page"+ str(cnt) + "["+ str(imgx)+ "] = ")
            f.write('{')
        for x in page:
            f.write(x)
            f.write(',')
        if flag_ino_data:
            f.write('};\n')
        else:
            f.write('\n')
        cnt = cnt + 1
    if flag_ino_data:
        f.write("uint8_t *pages["+str(cnt)+"] = {")
        for i in range(cnt):
            f.write("page" + str(i) + ", ")
        f.write("};\n")
        f.write("//====== data end ======\n")
    f.close()

def main():
    argv = sys.argv
    argc = len(argv)
    imagename = ""
    if (argc < 2):
        print("Usage: python imglcdnize.py sample.image -options")
        quit()
    else:
        for option in argv:
            if option[0] == '-':
                readoption(option)
    imgname = argv[1]
    img = shirokuro(Image.open(imgname))
    (imgx, imgy) = img.size
    pages = lcdnize(img, imgx, imgy)
    save(pages, imgname + ".lpi", imgx)


if __name__ == '__main__':
    main()
