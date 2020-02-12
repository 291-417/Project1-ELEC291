import time
import serial
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, math
import msvcrt
if msvcrt.kbhit():
        key_stroke = msvcrt.getch()
        print(key_stroke)

        
ser = serial.Serial(
    port = 'COM5',
    baudrate = 115200,
    parity = serial.PARITY_NONE,
    stopbits = serial.STOPBITS_TWO,
    bytesize = serial.EIGHTBITS
    )
ser.isOpen()



xsize=100

def data_gen():
    t = data_gen.t
    while True:
        t+=1
        strvar = ser.readline()
        strvar = float(strvar.decode('utf-8'))*(410/1023) - 273.15
        print(strvar)
        val=strvar
        yield t, val
        
def run(data):
    # update the data
    global print_list
    global ave_delta_temp
    global ann_delta_t
    global last_temp_print
    t,y1 = data
    if t>-1:
        xdata.append(t)
        ydata1.append(y1)
        if t>xsize: # Scroll to the left.
            ax.set_xlim(t-xsize, t)
        line1.set_data(xdata, ydata1)
        temp_max = 250
        temp_grad = (abs(y1%temp_max)) / temp_max
        red_val = temp_grad
        green_val = 1 - red_val
        
        if(len(ydata1) % 30 == 0) or (y1 > last_temp_print + 10) or (y1 < last_temp_print - 10) :
            last_temp_print = y1
            ax.annotate('(%s s , %.2f °C)' %(t, y1), xy= (t, y1), textcoords = 'data',color=(red_val,green_val,0.0))

        if ann_delta_t is not None:
               ann_delta_t.remove()
               
        ann_delta_t = plt.text(5, 5, 'Δt:%.2f' %(ave_delta_temp))
        
        if len(xdata) >= 10:
            temp_list = ydata1[t-5:t]
            #print(temp_list)
            #print(len(temp_list))
            #print(sum(temp_list))
            #print(print_list)
            #print(ave_delta_temp)
            delta_temp1 = (ydata1[t]-ydata1[t-1]) / (xdata[t] - xdata[t-1])
            delta_temp2 = (ydata1[t]-ydata1[t-2]) / (xdata[t] - xdata[t-2])
            delta_temp3 = (ydata1[t]-ydata1[t-3]) / (xdata[t] - xdata[t-3])
            ave_delta_temp = ( delta_temp1 + delta_temp2 + delta_temp3 ) / 3
            
            list_ave = sum(temp_list) / len(temp_list)
            if list_ave >=150 and ave_delta_temp <= -2 and print_list[4] == 0:
                ax.annotate("Cooling Down", xy=(t, y1), xycoords=ax.transData,xytext=(t, y1-125), textcoords=ax.transData,arrowprops=dict(arrowstyle="->"), bbox=dict(boxstyle="round", fc='tab:blue'))
                print_list[4] =1
            if list_ave >= 200 and ave_delta_temp >= 1 and print_list[3] == 0:
                ax.annotate("Entering Reflow", xy=(t, y1), xycoords=ax.transData,xytext=(t, y1-125), textcoords=ax.transData,arrowprops=dict(arrowstyle="->"), bbox=dict(boxstyle="round", fc='tab:red'))
                print_list[3] =1
            if list_ave >= 150 and ave_delta_temp >= 2 and print_list[2] == 0:
                ax.annotate("Ramp to Peak", xy=(t, y1), xycoords=ax.transData,xytext=(t, y1-125), textcoords=ax.transData,arrowprops=dict(arrowstyle="->"), bbox=dict(boxstyle="round", fc='tab:green'))
                print_list[2] =1
            if list_ave >= 145 and print_list[1] == 0:
                ax.annotate("Entering Preheat Soak", xy=(t, y1), xycoords=ax.transData,xytext=(t, y1-125), textcoords=ax.transData,arrowprops=dict(arrowstyle="->"), bbox=dict(boxstyle="round", fc='tab:gray'))
                print_list[1] =1
            if list_ave >= 30 and ave_delta_temp>= 2 and print_list[0] == 0:
                ax.annotate("Ramp to Soak", xy=(t, y1), xycoords=ax.transData,xytext=(t, y1+125), textcoords=ax.transData,arrowprops=dict(arrowstyle="->"), bbox=dict(boxstyle="round", fc="w"))
                print_list[0] = 1
                
    return line1

def on_close_figure(event):
    sys.exit(0)
    

data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw=2)
#ax.set_ylim(-100, 100)
ax.set(title="Tempurature over Time", xlabel="Time (seconds)", ylabel="Tempurature (deg C)")
ax.set_xlim(0, xsize)
ax.grid()
xdata, ydata = [], []

# Important: Although blit=True makes graphing faster, we need blit=False to prevent
# spurious lines to appear when resizing the stripchart.
ani = animation.FuncAnimation(fig, run, data_gen, blit= False, interval=100, repeat=False)
plt.show()
