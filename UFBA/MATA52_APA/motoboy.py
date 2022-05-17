#criar um array ij onde i representa o item i e j representa a capacidade de peso

def MotoboySolver(P,Nitem,arrayItems,W):
    peso = P 
    items = Nitem
    item = arrayItems 
    w = W 
    matrix = [[0 for C in range(w+1)] for R in range(items+1)]
    for i in range(1,items+1):
        for j in range (1,w+1):
            maxValWithCurr = 0
            maxValWithoutCurr = matrix[i-1][j]
            if (j >= peso[i-1]):
                maxValWithCurr = item[i-1]
                remainingCap = j - peso[i-1]
                maxValWithCurr = maxValWithCurr + matrix[i-1][remainingCap] 
            matrix[i][j] = max(maxValWithoutCurr,maxValWithCurr)
    #print(matrix)
    return matrix[Nitem][W]

while True:
    case = int(input())
    if case == 0:
        break
    w = int(input())
    item = []
    peso = []
    items = case
    for casos in range(case):
        catchString = input()
        catchString = catchString.split(" ")
        catchString = list(map(int, catchString))
        #print(catchString)
        item.append(catchString[0])
        peso.append(catchString[1])
    resultado = MotoboySolver(peso, len(item), item, w)
    print(str(resultado) + " min.")

    

