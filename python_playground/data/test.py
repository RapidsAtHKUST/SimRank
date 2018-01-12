def f(x):
    return x * x


if __name__ == '__main__':
    a = 0
    while True:
        while a < 100:
            a += 1
            if a > 50:
                break
        print("main loop")
        print(a)
        input()
