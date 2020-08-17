import scipy as sp
import matplotlib.pyplot as plt
from scipy.optimize import fsolve

filename = "data/ГИСМЕТЕО.csv"
column = 2

data = sp.genfromtxt(filename, delimiter=';', skip_header=1)

y = data[:,column][::2]
x = [x for x in range(1, len(y))]

plt.figure(figsize=(8, 6))
plt.title(filename)
plt.xlabel("x")
plt.ylabel("y")
plt.autoscale(tight=True)

y = y[1:]

plt.scatter(x, y)

fx = sp.linspace(x[0], x[-1], 1000)
fp = sp.polyfit(x, y, 5, full=True)[0]
f = sp.poly1d(fp)

plt.plot(fx, f(fx), linewidth=2)

plt.grid()
plt.show()
