from typing import List


currLine = ""
currLabel = ""


"""
    This funt. takes a list of string that represents each line of a sprite.
    Any char that is not a space will be considered as `ON` (it means its bit
    will be set)
"""
def spriteToHex(spriteLines: List[str]) -> List[int]:
    ret = []
    
    for l in spriteLines:
        tmp:int = 0
        for i in range(8):
            if l[i] != ' ':
                tmp |=  (0x1 << i)
        ret.append(0xFF & tmp)
    return ret

def getLabelAddr(name: str, labels: dict) -> int:
    for l in labels:
        if l == name: return labels[l][0]
    return -1

def printBuffer(buffer):
    addr = 0x200
    for i in range(len(buffer)):
        print("$%s %s" % (hex(addr), hex(buffer[i])))
        addr +=1

def printError(line: int, msg:str, lineContent:str):
    print("Line", line)
    print(lineContent)
    print(msg)

def setInstAndLabel(i:str, l:str):
    global currLine 
    global currLabel
    currLine = i
    currLabel = l


def getInstOps(instLine: str, label: str):
    splitInst = instLine.split(' ', 1)
    if len(splitInst) <= 1:
        print("Can't parse this inst. : \n %s \n in label %s" % (instLine, label))
        exit()
    # print(splitInst)
    return [op.replace(' ', '') for op in splitInst[1].split(',')]
    
"""
    Take an operand as string and return the register number that is in.
    op should be formatted this way : 'v0'...'v15'
"""
def getRegNo(op: str) -> int:
    tmp = 0
    try:
        tmp = int(op[1:])
    except ValueError:
        print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct." % (currLine, currLabel))
        exit()

    if tmp < 0 or tmp > 0xF:
        print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct. Register out of bounds." % (currLine, currLabel))
        exit()

    return tmp

"""
    This function takes a string that represents an hex int.
    The specifier can be specified with the _hexdel_ param.
    It returns an int.
"""
def getIntFromHexStr(hexS: str="", hexDel="$"):
    hexS = hexS.replace(hexDel, '0x')
    try:
       hexS = int(hexS, 16)
    except ValueError:
        print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct. Can't parse this value." % (currLine, currLabel))
        exit()

    return hexS


"""
    This Function parses LD CPU instruction and returns the two bytes
    that represents it in memory.
    Takes an STR instruction (such as 'ld v1, v2') and return
    two bytes representing it.

    - Bytes returned are in a most significant byte first order, keep that in mind.
"""
def parseInst_LD(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)


    instLine = instLine.lower()
    operands = getInstOps(instLine, label)
    if len(operands) != 2:
        print("Error with LD insctuction : %s \nIn label %s" %(instLine, label))
        exit()
    op1:str
    op2:str
    op1, op2 = operands
    d1, d2  = 0,0
    byte1: int = 0
    byte2: int = 0
    # Registers load
    # LD vx, vy 8xy0
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | d1
        byte2 = 0xFF & d2
    # LD vx, bb 6xbb
    elif op1.startswith('v') and op2.startswith(hexDel):
        d1 = getRegNo(op1)
        d2 = getIntFromHexStr(op2, hexDel=hexDel)
        byte1 = 0x80 | (d1 & 0xF0)
        byte2 = 0xFF & d2
    # LD I, bbb
    elif op1 == "i":
        d1 = getIntFromHexStr(op2, hexDel)
        byte1 = 0xA0 | (0xF00 & d1)
        byte2 = 0x0FF & d1
    
    # LD vx, dt Fx07
    elif op2 == "dt":
        d1 = getRegNo(op1)
        byte1 = 0xF0 | (d1 & 0x0F)
        byte2 = 0x07

    # LD vx, k Fx01
    elif op2 == "k":
        d1 = getRegNo(op1)
        byte1 = 0xF0 | (d1 & 0x0F)
        byte2 = 0x01

    # LD DT, vx Fx15
    elif op1 == "dt":
        d2 = getRegNo(op2)
        byte1 = 0xF0 | (d2 & 0x0F)
        byte2 = 0x15

    # LD ST, vx Fx18
    elif op1 == "st":
        d2 = getRegNo(op2)
        byte1 = 0xF0 | (d2 & 0x0F)
        byte2 = 0x18
    
    # LD F, vx Fx29
    elif op1 == "f":
        d2 = getRegNo(op2)
        byte1 = 0xF0 | (d2 & 0x0F)
        byte2 = 0x29

    # LD B, vx Fx33
    elif op1 == "b":
        d2 = getRegNo(op2)
        byte1 = 0xF0 | (d1 & 0x0F)
        byte2 = 0x33

    # LD [i], vx Fx55
    elif op1 == "[i]":
        d2 = getRegNo(op2)
        byte1 = 0xF0 | (d1 & 0x0F)
        byte2 = 0x55

    # LD vs, [i] Fx65
    elif op2 == "[i]":
        d1 = getRegNo(op1)
        byte1 = 0xF0 | (d1 & 0x0F)
        byte2 = 0x65
    else:
        print("Well I can't parse this LD instruction \n\t%s\n\t in label : %s" % (instLine, label))
        exit()

    return byte1, byte2