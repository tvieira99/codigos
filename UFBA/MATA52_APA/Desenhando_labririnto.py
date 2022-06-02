import math
Nentradas = int(input())
def DFS(itemBuscado):
    if (visited[itemBuscado] == 1):
        return
    #print(f"Estou procurando {itemBuscado}")
    visited[itemBuscado] = 1
    for ent in entrada: 
       # print(f"analisando: {entrada[i]}")
        if (itemBuscado == ent[0] or itemBuscado == ent[1]):
           # print(f"achou item buscado ent:{ent[1]} visited:{visited[ent[1]]}")
            if (itemBuscado == ent[0] and visited[ent[1]] != 1):
                vertices.append(ent)
               # print(f"inserted: {ent}")
                vertices.append(ent)
                DFS(ent[1])
            if (itemBuscado == ent[1] and visited[ent[0]] != 1):
                vertices.append(ent)
            #    print(f"inserted: {ent}")
                vertices.append(ent)
                DFS(ent[0])

for i in range (0, Nentradas):
    entrada = []
    visited = []
    vertices = []
    Nindex = int(input())
    Narver = input()
    Narver = list(map(int, Narver.split(" ")))
    a,v = Narver
    
    for i in range (0, a):
        visited.append(0)
    
    for i in range(0,v):
        vert = input()
        vert = tuple(map(int, vert.split(" ")))
        #print(vert)
        entrada.append(vert)
    DFS(Nindex)
  #  print(entrada)
  #  print(visited)
    print(len(vertices))

