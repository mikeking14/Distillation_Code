from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
import matplotlib.pyplot as plt
#------------------------------------------------------------------#

# Variable to Change
distillation_run_number = 35

#------------------------------------------------------------------#
file_path = "/Users/michaelking/Documents/PlatformIO/Projects/Distillation_Code/DataProcessing/data/Distilation" + str(distillation_run_number) + '.txt'
plt.style.use('fivethirtyeight')

def animate(i):
    data = pd.read_csv(file_path)
    matrix_data = data.values
    x_value = matrix_data[:,0]
    y1 = matrix_data[:,1]
    y2 = matrix_data[:,2]

    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    color = 'tab:red'
    ax1.set_xlabel('time (s)')
    ax1.set_ylabel('exp', color=color)
    ax1.plot(x_value, y1, color=color)
    ax1.tick_params(axis='y', labelcolor=color)

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
    color = 'tab:blue'
    ax2.set_ylabel('sin', color=color)  # we already handled the x-label with ax1
    ax2.plot(x_value, y2, color=color)
    ax2.tick_params(axis='y', labelcolor=color)

#    plt.cla()
#    plt.legend(loc='upper left')

ani = FuncAnimation(plt.gcf(), animate, interval=1000)

plt.tight_layout()
plt.show()
