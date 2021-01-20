from typing import List

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

