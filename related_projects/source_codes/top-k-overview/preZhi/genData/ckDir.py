import sys
if __name__ == '__main__':
    gname = sys.argv[1]
    isDirected = False
    if gname == '':
        print 'Usage: checkDirected <file>'
        sys.exit(1)
    else:
        with open(gname) as gdata:
            for line in gdata:
                s = line.split()
                if int(s[0].strip()) > int(s[1].strip()):
                    print s
                    isDirected = True
                    break
        print 'isDirected:' + str(isDirected)
    
