memo = {0:0,1:1}

def fibonacci_of(n):
    if n in memo:
        return memo[n]
    memo[n] = fibonacci_of(n-1) + fibonacci_of(n-2)
    return memo[n]
print(fibonacci_of(50))
