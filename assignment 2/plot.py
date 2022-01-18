import numpy as np
import sys
import matplotlib.pyplot as plt
congestionWindowValues = np.loadtxt(sys.argv[1])
K_i=sys.argv[2]
K_m=sys.argv[3]
K_n=sys.argv[4]
K_f=sys.argv[5]
P_s=sys.argv[6]
xpoints = np.arange(1, congestionWindowValues.shape[0]+1)
ypoints = congestionWindowValues

plt.plot(xpoints, ypoints)
plt.xlabel("Update number")
plt.ylabel("Congestion Window size in kB")
plt.title("Congestion Window size vs Update number\n $K_i$ = "+K_i+" $K_m$ = "+K_m+" $K_n$ = "+K_n+" $K_f$ = "+K_f+" $P_s$ = "+P_s)
plt.savefig(sys.argv[1]+"plt.png")