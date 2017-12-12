import math

def T_BLP(c, d, r_sum):
    '''
    Worst case time complexity of backward local push
    '''
    cd = c / d
    print("target sum",(1 - r_sum) / (1-c) )
    print("limite sum", 1 / (1-cd))
    t = d * math.log(1 - (1-cd) * (1 - r_sum) / (1-c), cd)
    return t

if __name__ == "__main__":
    c = 0.6
    d = 10
    r_sum = 0.1
    print(T_BLP(c, d, r_sum))
