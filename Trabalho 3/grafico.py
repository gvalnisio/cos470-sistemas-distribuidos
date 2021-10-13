import numpy as np
from pylab import *
from matplotlib import pyplot

t0 = 1634081451857978923 - 1634081433852308042
t1 = 1634089960682525182 - 1634089860549750353
t2 = 1634091105882516424 - 1634091072718593766
t3 = 1634091377285930580 - 1634091367478821247


n = [2, 62, 126, 254]
t = [18.01, 100.13, 33.16, 0.981]

pyplot.plot(n, t, color = "magenta", linewidth = 1.0)
pyplot.xticks(n)
pyplot.yticks(t)
pyplot.xlabel("Número 'n' de processos")
pyplot.ylabel("Tempo (s)")
pyplot.grid(True)

pyplot.savefig("grafico.png")
pyplot.show()