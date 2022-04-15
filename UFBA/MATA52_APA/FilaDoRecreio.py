def insertionSort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i-1
        while j >= 0 and key > arr[j] :
                arr[j + 1] = arr[j]
                j -= 1
        arr[j + 1] = key

casosTestes = int(input())

for i in range (0,casosTestes):
    nCasos = int(input())  #ignora a entrada do npumero já que pé python e a gente n precisa
    lAlunos = str(input())
    lAlunos = lAlunos.split(" ")
    lAlunos = list(map(int, lAlunos))
    lACopia = lAlunos.copy()
    insertionSort(lAlunos)
    nTrocas = 0
    for j in range (0,nCasos):
        if lAlunos[j] == lACopia[j]:
            nTrocas = nTrocas + 1
    print(nTrocas)
