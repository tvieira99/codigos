from sys import stdin
for line in stdin:
    num_dias = int(line) 
    custo_por_dia = int(input())
    lucro = []
    for i in range (0,num_dias):
        lucro.append(int(input()))
    maxValue = 0
    for i in range (0,num_dias):
        sumTotal = 0
        #    print(f"{lucro[i]}")
        for j in range (i, len(lucro)):
            sumTotal = sumTotal + lucro[j] - custo_por_dia
	#        print("\t" + str(sumTotal))
            if(sumTotal > maxValue):
                maxValue = sumTotal
    print(maxValue)
