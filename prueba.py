import matplotlib.pyplot as plt
from datetime import datetime
from matplotlib import pyplot
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
import random 
from tkinter import *
from tkinter import ttk

import matplotlib.animation as animacion


DEVICE_FILE = "/dev/ASMAN_driver"

def write(channel):    
    
    file = open(DEVICE_FILE, "w")
    file.write(channel)
    file.close()


def read_sensor():
    #cat
    file = open(DEVICE_FILE,"r")
    value = file.read()
    file.close()

    return value

#objeto grafico
class Index:
        #inicializacion
        def __init__(self):
            self.figA = plt.figure()
            self.figA.set_size_inches(10,8)
            
            
            self.x_data = []
            self.y_data = []
            
            self.line, = pyplot.plot_date(self.x_data, self.y_data, '-')
            plt.style.use('ggplot')
        
            self.anim = FuncAnimation(self.figA, 
                                            self.animada,   
                                            interval=1000,
                                            blit=True,frames=20, repeat=False)  
      
        #esta funcion se llama en en cada step
        def animada(self,i):
            valor1 = float(read_sensor())
            print("Valor: ",valor1,"\n")
            time1 = datetime.now()
            self.x_data.append(time1)
            self.y_data.append(valor1)
            self.line.set_data(self.x_data, self.y_data)
            self.figA.gca().relim()
            self.figA.gca().autoscale_view()
            return self.line,
        
        #grafica la señal indicada por channel
        def graficar(self,channel): 
            write(channel)
            plt.show()
            




while(True):
    channel=input("seleccione el canal \"a\" o \"b\": ")
    while (channel!='a' and channel!='b' and channel!='exit' ):
        channel=input("el channle "+channel+" no existe, elija \"a\", \"b\" o \"exit\" para salir: ")
    if(channel=="exit"):
            break    
    grafica=Index()
    grafica.graficar(channel)


    

  
    
