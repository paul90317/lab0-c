import matplotlib.pyplot as plt

def hist(samples :list[int], n :int, **kargv):
    mx = max(samples)
    mi = min(samples)
    intv :float = float((mx - mi) / n)
    x :list[float] = [mi + intv / 2]
    while 1:
        x.append(x[-1] + intv)
        if x[-1] >= mx:
            break
    
    y = [0 for _ in range(len(x))]
    for s in samples:
        y[int((s - mi) / intv)] += 1
    
    plt.bar(x, y, **kargv, width = intv)

data = ([], [])
with open("out", "r") as f:
    for li in f.readlines():
        if not li.startswith("dut"):
            continue
        c, t = li[4:].split(" ")
        data[int(c)].append(int(t))

plt.figure()
plt.xlabel("cycles")
plt.ylabel("times")
print(len(data[0]), len(data[1]))
hist(data[0], 2000, label = "fix")
hist(data[1], 2000, label = "random")
# plt.xlim(0, 1500)
plt.legend()
plt.show()