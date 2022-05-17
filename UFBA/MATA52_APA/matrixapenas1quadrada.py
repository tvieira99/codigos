R = 6
C = 5
 
def matrizQuadrada(M):
    Max = 0
    #Construir a matrix pra ser substituída com todos os valores iniciados em 0 
    S = [[0 for col in range(C)]for row in range(R)]
    # Iterando sobre a Matriz M
    for i in range(R):
        for j in range(C):
            # Calcula a entrada 
            Num = M[i][j]
            if(Num):
                if(j):
                    Num = 1 + min(S[i][j - 1],S[i-1][j], S[i-1][j-1])
                    S[i][j] = Num
            # Save the last entrie and add the new one
            else:
                S[i][j] = 0
            # Keep track of the max square length
            Max = max(Max, Num)
    # Print the square
    print(f"A matriz é formada por {Max} X {Max} ")
         
 
# Driver code
M = [[0, 1, 1, 0, 1],
                    [1, 1, 0, 1, 0],
                    [0, 1, 1, 1, 0],
                    [1, 1, 1, 1, 0],
                    [1, 1, 1, 1, 1],
                    [0, 0, 0, 0, 0]]
                      
matrizQuadrada(M)
 
