def prob(n1, n2, at):
    if (at == 3):
        return (n1)/(n1+n2)
    else:
        prob = 6 - at
        prob = (prob/6)/(at/6)
        p = (1 - pow(prob,n1))/(1 - pow(prob,n1+n2))
        return p
while True:
    lista = input()
    if lista == "0 0 0 0":
        break
    #pega os resultado e os coloca em uma lista
    lista= lista.split(" ")
    lista = list(map(int, lista))
    tst = lista[0]
    lista[0] = 0
    while tst > 0:
        tst = tst - lista[3]
        lista[0] = lista[0] + 1
    tst = lista[1]
    lista[1] = 0
    while tst > 0:
        tst = tst - lista[3]
        lista[1] = lista[1] + 1
    resultado = prob(lista[0], lista[1], lista[2])
    resultado = resultado * 100
    print("{:.1f}".format(resultado))
