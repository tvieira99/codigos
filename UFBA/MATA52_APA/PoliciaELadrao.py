global achou
achou = False
def PEL(entrMatrix,i,j):
    if j > 4 or i > 4 or i < 0 or j < 0:
        return
    entrMatrix[i][j] = 2
#    print(entrMatrix)
    if i == 4 and j == 4:
        global achou
        achou = True
    #casos a checar
    if j <= 3 and j >= 0 and i >= 0 and i <= 4 and entrMatrix[i][j+1] == 0:#CAMINHAR PARA A DIREITA
        PEL(entrMatrix, i, j+1) 
    if i <= 3 and i >= 0 and j <= 4 and j >= 0 and entrMatrix[i+1][j] == 0:#CAMINHAR PARA BAIXO 
        PEL(entrMatrix, i+1, j)
    if i >= 1 and i <= 4 and j <= 4 and j <= 0 and entrMatrix[i-1][j] == 0:#CAMINHAR PARA CIMA
        PEL(entrMatrix, i-1, j) 
    if j >= 1 and j <= 4 and i >= 0 and i <= 4 and entrMatrix[i][j-1] == 0:#CAMINHAR PARA A ESQUERDA 
        PEL(entrMatrix, i, j-1) 
    entrMatrix[i][j] = 0

nTestes = int(input())
for i in range(0, nTestes):
    entrMatrix = []
    nAdicionado = 0
    while nAdicionado < 5:
        linha = input()
        if linha == '' or linha == chr(0x0a) or linha == chr(0x0d) or linha == chr(0x20):
            continue
        linha = linha.split(chr(0x20))
        for k in linha:
            if k == '':
                linha.remove(k)
        linha = list(map(int,linha))
        entrMatrix.append(linha)
        nAdicionado = nAdicionado + 1
    achou = False
    PEL(entrMatrix,0,0)
    if achou:
        print("COPS")
    else:
        print("ROBBERS")
