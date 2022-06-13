#FAZENDO A QUESTÃO 2
import random
X = ['a', 'b', 'c', 'd', 'e']
F = [['a', 'e', 'e'], ['c'], ['a', 'b'], ['d','e'], ['b','c'], ['b', 'c', 'a']] 
dictF = {'a': False, 'b': False, 'c': False, 'd': False, 'e': False}
uniqueElements = []
uniqueEntryNumber = [] 
resp = []
for fEntry in F:
    tst = dictF.copy()
    uniqnumb = 0
    for i in fEntry:
        if(tst[i] != True):
            tst[i] = True
            uniqnumb += 1
    uniqueEntryNumber.append(uniqnumb)
    uniqueElements.append(tst)

def condition():
    for i in dictF:
        if (dictF[i] == False):
            return True
    return False 


while (condition()):
    indexMaiorNumElemUnicos = 0
    maxUniqElem = 0
    for k in range(0, len(uniqueEntryNumber)):
        if (uniqueEntryNumber[k] > maxUniqElem):
            indexMaiorNumElemUnicos = k 
            maxUniqElem = uniqueEntryNumber[k]
    aux = []
    
    for i in uniqueElements[indexMaiorNumElemUnicos]:
        if(uniqueElements[indexMaiorNumElemUnicos][i] == True):
         aux.append(i)
    resp.append(aux)
    
    for i in uniqueElements[indexMaiorNumElemUnicos]:
        if(uniqueElements[indexMaiorNumElemUnicos][i] == True):
            dictF[i] = True

    for i in dictF:
        for j in range (0,len(uniqueElements)):
            if(dictF[i] == True and uniqueElements[j][i] == True):
                uniqueElements[j][i] = False
                uniqueEntryNumber[j] -= 1
print(resp)
