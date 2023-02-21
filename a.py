import matplotlib.pyplot as plt
import numpy as np

def show(filename,name):
    with open(filename,'r') as f:
        R=[int(s)for s in f.readlines()]
        nR=np.array(R)
        mR=0
        MR=2000
    d=(MR-mR)/100
    y=[i for i in range(100)]
    x=[mR+xx*d for xx in range(100)]
    for r in R:
        dd=int((r-mR)/d)
        if(dd>=100):
            y[99]+=1
        elif(dd<0):
            y[0]+=1
        else:
            y[dd]+=1
    plt.plot(x,y,label=name)
    plt.legend()
    plt.xticks([i for i in range(0,800,50)])
    plt.xlim(0,800)
    plt.xlabel('cycle')
    plt.ylabel('frequency')
    plt.grid()
    # plt.ylim(0,1)

plt.figure()
show('./a.txt','q_insert_head')
show('./b.txt','q_remove_head')
plt.show()