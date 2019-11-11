from random import randrange

import matplotlib.pyplot as plt
import numpy as np
from drawnow import drawnow

def make_fig():
    plt.plot(x[-5:], y[-5:])  # I think you meant this

plt.ion()  # enable interactivity
fig = plt.figure()  # make a figure
plt.xlim([10,0])
plt.ylim([-1,10])

y = []
x = []

for i in range(1000):
    temp_y = randrange(0,10)
    x.append(i)
    y.append(temp_y)  # or any arbitrary update to your figure's data
    i += 1
    drawnow(make_fig)
