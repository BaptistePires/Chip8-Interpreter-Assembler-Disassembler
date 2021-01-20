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
            buf = l
            if(buf.replace(' ', '') != '\n'): srcLines.append(l[:-1])
except IOError:
    print("Error while reading input file %s, can't open." % argv[1])
    exit()

# Now we parse source file
hexDel:str = "$"
lineSplit = List[str]
"""
    Sprites will be stored in a 2dim array:
    For each sprite, there is one entry structured as follow:
        entry 0         : sprite's name
        entry 1         : sprite's height
        entry 2..height : 8bit unsigned int that represents the 8bits for a pixel line.
"""
sprites = []
countSprites: int = 0

"""
    List that will contains pairs.
    [label's name, addr]
"""
labels = []

# Test segment size in bytes.
instructions = []

skip = [False, 0]
for i in range(len(srcLines)):
    l = srcLines[i]
    if skip[0] == True:
        skip[1] = skip[1] - 1
        if skip[1] == 0:
            skip[0] = False
        continue
    if(l == '\n'): continue

    if l[0] == '#':
        if "HEXDEL" in l:
            lineSplit = l.split(' ')
            if lineSplit[1] in ['$', '0x']:
                hexDel = lineSplit[1]

    elif l[0] == '.':
        ## Handle sprite
        if ".sprite" in l:
            label, name, height, sType = [w.replace(' ', '') for w in l.split(' ')]
            
            height = int(''.join([char for char in height if char.isnumeric()]))
            sprites.append([name, height])
            spriteLst = [srcLines[j].replace('"', '') for j in range(i + 1, i + 1 + height)]
            if sType == "s":
                sprites[countSprites] += spriteToHex(spriteLst)
            else :sprites[countSprites] += [int(x, 16) & 0xFF for x in spriteLst]
            countSprites +=1
            skip = [True, height] 
    else:
        if l.endswith(':'):
            labels.append((l[:-1], i))
        else:
            print(l)
            instructions.append(l)    

print("Sprites : ", sprites)
print("Labels : ",  labels)              
print("Inst :", instructions)




