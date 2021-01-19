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
        for i, char in enumerate(l):
            tmp <<= 1
            if char != ' ':
                tmp |= 1 << i
        
        ret.append(bin(0xFF & tmp))
    return ret

