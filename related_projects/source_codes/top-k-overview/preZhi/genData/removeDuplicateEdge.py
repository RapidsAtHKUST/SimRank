import sys
if __name__ == '__main__':
    gname = sys.argv[1]
    isDirected = False
    if gname == '':
        print 'Usage: removeDuplicate <file>'
        sys.exit(1)
    else:
        outfname = gname + '.rd'
        outf = open(outfname, 'w')
        s = set()
        for line in open(gname):
            if line in s:
                print 'duplicate edge', line
            else:
                s.add(line)
        for x in s:
            outf.write(x)
        outf.close()

