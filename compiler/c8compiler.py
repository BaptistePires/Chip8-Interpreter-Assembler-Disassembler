from sys import argv
from textwrap import dedent
from typing import List

from utils import *

MEM_START = 0x200

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
            if(buf.replace(' ', '') != '\n'): srcLines.append(l.replace('\n', ''))
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
    l = srcLines[i].lower()
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
            else:
                sprites[countSprites] += [int(x.replace(hexDel, ' '), 16)&0xFF for x in spriteLst]
            # else :sprites[countSprites] += [int(x, 16) & 0xFF for x in spriteLst]
            countSprites +=1
            skip = [True, height] 
    else:
        if l.startswith('_'):
            label = l
            if label.endswith(':'): label = label[:-1]
            if len(label) <= 1:
                printError(i, "You have to give a name to your label, not", srcLines[i])
                exit()
            labels.append((label, i))
        else:
            instructions.append(l)    

print("Sprites : ", sprites)
print("Labels : ",  labels)              
print("Inst :", instructions)
totalSpriteBytes = sum([len(s) - 2 for s in sprites])

# 
romBuffer = []
addr: int = MEM_START

# write jump after sprites 
# +2 for curr inst & need to go 'behind' sprites
romBuffer.append((addr + totalSpriteBytes + 2) & 0xFF)
romBuffer.append((((addr + totalSpriteBytes + 2) & 0xF00) >> 8)| 0x10)
addr += 2
# We store sprite directly at rom's start
# Now the first sprite index will be his memory addr

for i, s in enumerate(sprites):
    sprites[i].insert(0, addr)
    for y in range(3, len(s)):
        romBuffer.append(0xFF & s[y])
        addr+=1

print("Sprites : ", sprites)
print([hex(x) for x in romBuffer])

"""
    Array containing:
        - 0    : label's addr
        - 1    : label's name
        - 2..n : label's instructions
"""
labelsWithInst = {}
labelCount: int = 0
currLabel: str = ""
# Not that good but we'll work with small files so it's ok i guess
for i in range(len(srcLines)):
    l = srcLines[i]
    l = dedent(l)
    if len(l) == 0: continue

    if not l.startswith('_') :
        if len(labelsWithInst) == 0:continue
        else: 
            labelsWithInst[currLabel].append(l)
            addr += 2
            
    else:
        currLabel = l
        labelsWithInst[l] = [addr]
        labelCount+=1

print(labelsWithInst)
# Now process instructions and store them :)


byte1: int = 0x0
byte2: int = 0x0

for l in labelsWithInst:
    for i in range(1, len(labelsWithInst[l])):
        inst: str = labelsWithInst[l][i].lower()
        print("in:"+inst)
        # SYS nnn
        if(inst.startswith("sys")):
            split = inst.split(' ')
            byte1 = ((int(split[1]) & 0xF00) >> 8 & 0xFF)
            byte2 = (int(split[1]) & 0xFF)

        # RET
        elif inst.startswith("ret"):
            byte1 = 0x00
            byte2 = 0xFF

        # JP & JP0
        elif inst.startswith("jp"):
            split = inst.split(' ')
            if split[0] == "jp0":pass
            byte1 = (((int(split[1]) & 0xF00) >> 8) & 0xFF)
            byte2 = (int(split[1]) & 0xFF)

        # CALL
        elif inst.startswith('call'):
            split = inst.split(' ')
            byte1 = (((int(split[1]) & 0xF00) >> 8) & 0xFF)
            byte2 = (int(split[1]) & 0xFF)
        # All LDs
        elif(inst.startswith("ld")):
            byte1, byte2 = parseInst_LD(inst, l, hexDel)
        #
        else:
            print("Unknow instruction : %s" % inst)
            print("Leaving ...")
            break

        romBuffer.append(byte2 & 0xFF)
        romBuffer.append(byte1 & 0xFF)
        addr += 2

        

printBuffer(romBuffer)
