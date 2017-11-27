#!/data/data/u.r/bin/micropython

import os,sys

rep = os.popen("grep 'Fatal signal' /mnt/secure/lc.txt").read().strip()
print(rep)
print("-"* len(rep))
lines = []
for line in os.popen("grep '#..  pc ' /mnt/secure/lc.txt"):
    line = line.strip().split('  pc ')[1].split('(')[0].strip()
    lines.append( line.split('  ') )

lines.reverse()
LAST= ''
LASTF=''
depth=-1
for line in lines :
    if LAST!=line[1]:
        LAST=line[1]
        depth+=1
    res= os.popen('addr2line -i -p -C -f -e %s %s' % (line[1],line[0]) ).read()
    if res.count('??'):
        continue

    func,code = res.strip().split(' at ')

    if LASTF!=code:
        LASTF = code
        #print(' '*depth, end='')
        depth+=1
        print()
        if sys.argv[-1].count('/'):
            print(code.split(sys.argv[-1])[-1] )
        else:
            print(code)
    print(' '*depth, end='')
    print(func)
