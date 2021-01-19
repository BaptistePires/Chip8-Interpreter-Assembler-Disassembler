from sys import argv
from typing import List
from utils import *


srcLines: str = []
targetFile = "a.c8c"
lenArgv = len(argv)

# Parse args
if lenArgv < 2:
    print("Usage :\n\tpython3 %s filename [-o output]" % argv[0])
    exit()
elif lenArgv > 2:

    i: int
    for i in range(lenArgv):
        if argv[i] == "-o":
            if((i + 1) < lenArgv):
                targetFile = argv[i + 1]
            
# Read source file
try:
    with open(argv[1], "r") as srcFile:
        for l in srcFile.readlines():
            # Strip \n
            srcLines.append(l[:-1])
except IOError:
    print("Error while reading input file %s, can't open." % argv[1])
    exit()

# Now we parse source file
hexDel:str = "$"
lineSplit = List[str]
"""
    Sprites will be stored in a 2dim array:
    For each sprite, there is one entry structured as follow:
        entry 0    : height of sprite
        entry 1..n : 8bit unsigned int that represents the 8bits for a pixel line.
"""
sprites = []
countSprites: int = 0

for i in range(len(srcLines)):
    l = srcLines[i]

    if l[0] == '#':
        if "HEXDEL" in l:
            lineSplit = l.split(' ')
            if lineSplit[1] in ['$', '0x']:
                hexDel = lineSplit[1]

    elif l[0] == '.':
        ## Handle sprite
        if ".sprite" in l:
            label, height, sType = [w.replace(' ', '') for w in l.split(' ')]
            
            height = int(''.join([char for char in height if char.isnumeric()]))
            sprites.append([height])
            spriteLst = [srcLines[j] for j in range(i, i + height)]
            if sType == "s":
                print(spriteToHex(spriteLst))

            
            i+= height
            countSprites +=1

print(sprites)
              





