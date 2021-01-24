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
        print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct.\n" % (currLine, currLabel))
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
    if hexDel not in hexS:
        try:
            hexS = int(hexS)
            return hexS
        except ValueError:
            print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct. Can't parse this value.\n" % (currLine, currLabel))
            exit()

    hexS = hexS.replace(hexDel, '')
    try:
       hexS = int(hexS, 16)
    except ValueError as e:
        print("[ERR] : \n\tOperand of %s\n\tIn label : %s\nIs not correct. Can't parse this value.\n" % (currLine, currLabel))
        exit()

    return hexS

def printParseErr(msg: str, label:str, lineContent: str):
    print("[ERR] Message : %s\n\tIn label : %s\n\tLine content : %s\n\tLeaving..." % (msg, label, lineContent))
    exit()



 ##################################
# Here start instructions parsing. #
 ##################################
 


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
        byte2 = ((d2 & 0xF) << 4) | 0x00
    # LD vx, bb 6xbb
    elif op1.startswith('v'):
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


def parseInst_JP(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')
    if len(split) != 2:
        print("Well I can't parse this JP instruction \n\t%s\n\t in label : %s" % (instLine, label))
        exit()
    
    
    addr = getIntFromHexStr(split[1], hexDel)
    if addr > 0xFFF:
        print("You can't jump outside memory : \n\t%s\n\t in label : %s" % (instLine, label))
        exit()

    return ((addr & 0xF00) >> 8) | 0x10, addr & 0x0FF

def parseInst_CALL(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')
    if len(split) != 2:
        print("Well I can't parse this JP instruction \n\t%s\n\t in label : %s" % (instLine, label))
        exit()
    
    
    addr = getIntFromHexStr(split[1], hexDel)
    if addr > 0xFFF:
        print("You can't call outside memory : \n\t%s\n\t in label : %s" % (instLine, label))
        exit()

    return ((addr & 0xF00) >> 8) | 0x20, addr & 0xFF


def parseInst_SE(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SE inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0
    # 5xy0
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x50 | d1
        byte2 = (d2 << 4)
    # 3xbb
    else:
        d1 = getRegNo(op1)
        d2 = getIntFromHexStr(op2)
        byte1 = 0x30 | d1
        byte2 = 0xFF & d2

    return byte1, byte2

def parseInst_SNE(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SE inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0
    # 9xy0
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x90 | d1
        byte2 = d2 << 4
    # 4xbb
    else:
        d1 = getRegNo(op1)
        d2 = getIntFromHexStr(op2)
        byte1 = 0x40 | d1
        byte2 = 0xFF & d2

    
    return byte1, byte2


def parseInst_ADD(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SE inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # fx1e
    if op1 == "i" and  op2.startswith('v'):
        d1 = 0x00
        d2 = getRegNo(op2)
        byte1 = 0xF0 | d2
        byte2 = 0x1E
    # 8xy4
    elif op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0xF0 | d1
        byte2 = ((d2 & 0xF) << 4)| 0x04
    # 7xbb
    else:
        d1 = getRegNo(op1)
        byte1 = 0xF0 | d1
        byte2 = d2 & 0xFF

    return byte1, byte2


def parseInst_OR(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for OR inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy1
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x01
    else:
        printParseErr("Wrong OR operands, need OR vx, vy", label, instLine)

    return byte1, byte2

def parseInst_AND(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for AND inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy2
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x02
    else:
        printParseErr("Wrong AND operands, need AND vx, vy", label, instLine)

    return byte1, byte2

def parseInst_XOR(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for XOR inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy3
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x03
    else:
        printParseErr("Wrong XOR operands, need XOR vx, vy", label, instLine)

    return byte1, byte2

def parseInst_SUB(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SUB inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy5
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x05
    else:
        printParseErr("Wrong SUB operands, need SUB vx, vy", label, instLine)

    return byte1, byte2

def parseInst_SHR(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SHR inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy6
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x06
    else:
        printParseErr("Wrong SHR operands, need SHR vx, vy", label, instLine)

    return byte1, byte2

def parseInst_SUBN(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SUBN inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xy7
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x07
    else:
        printParseErr("Wrong SUBN operands, need SUBN vx, vy", label, instLine)

    return byte1, byte2

def parseInst_SHL(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SHL inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # 8xyE
    if op1.startswith('v') and op2.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        byte1 = 0x80 | (d1 & 0x0F)
        byte2 = ((d2 & 0xF) << 4) | 0x0E
    else:
        printParseErr("Wrong SHL operands, need SHL vx, vy", label, instLine)

    return byte1, byte2

def parseInst_JP0(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')
    if len(split) != 2:
        print("Well I can't parse this JP0 instruction \n\t%s\n\t in label : %s" % (instLine, label))
        exit()
    
    
    addr = getIntFromHexStr(split[1], hexDel)
    if addr > 0xFFF:
        printError("You can't jump outside memory", instLine, label)

    return ((addr & 0xF00) >> 8) | 0xB0, addr & 0x0FF

def parseInst_RND(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 3: printParseErr("Not enough operands for SHL inst", label, instLine)

    op1, op2 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2  = 0,0

    # cxbb
    if op1.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getIntFromHexStr(op2)
        byte1 = (d1 & 0xF) | 0xC0
        byte2 = d2 & 0xFF
    else:
        printError("Wrong format for instruction RDN", label, instLine)

    return byte1, byte2

def parseInst_DRW(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 4: printParseErr("Not enough operands for DRW inst", label, instLine)

    op1, op2, op3 = getInstOps(instLine, label)
    byte1:int=0
    byte2:int=0
    d1, d2, d3  = 0,0,0
    # dxyn
    if op1.startswith('v') and op2.startswith('v') and not op3.startswith('v'):
        d1 = getRegNo(op1)
        d2 = getRegNo(op2)
        d3 = getIntFromHexStr(op3)
        if d3 <= 0 or d3 > 8:
            printParseErr("Sprites must have a height between 1 and 8", label, instLine)
        byte1 = (d1 & 0xF) | 0xD0
        byte2 = ((d2 & 0xF) << 4) | (d3 & 0xF)
    else:
        printParseErr("Wrong DRW format need : DRW vx, vy, n", label, instLine)
        
    return byte1, byte2


def parseInst_SKP(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 2: printParseErr("Not enough operands for SKP inst", label, instLine)

    op1 = getInstOps(instLine, label)[0]
    byte1:int=0
    byte2:int=0
    d1 = 0
    # ex9e
    if op1.startswith('v'):
        d1 = getRegNo(op1)
        byte1 = 0xE0 | (d1 & 0xF)
        byte2 = 0x9E
    else:
        printParseErr("Wrong SKP format need : SKP vx", label, instLine)

    return byte1, byte2

def parseInst_SKNP(instLine: str, label:str, hexDel: str) -> (int, int):
    setInstAndLabel(instLine, label)
    split = instLine.split(' ')

    if len(split) < 2: printParseErr("Not enough operands for SKNP inst", label, instLine)

    op1 = getInstOps(instLine, label)[0]
    byte1:int=0
    byte2:int=0
    d1 = 0
    # exA1
    if op1.startswith('v'):
        d1 = getRegNo(op1)
        byte1 = 0xE0 | (d1 & 0xF)
        byte2 = 0xA1
    else:
        printParseErr("Wrong SKP format need : SKNP vx", label, instLine)

    return byte1, byte2
