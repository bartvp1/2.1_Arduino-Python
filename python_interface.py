from tkinter import *
from random import randint
import time

root = Tk()
root.title('Interface')
root.resizable(False, False)

canvas = Canvas(root, width=1200, height=600, bg='grey')  # 0,0 is top left corner
canvas.pack(expand=NO, fill=BOTH)


canvas.create_line(175, 600, 175, 0, width=1)
canvas.create_line(800, 600, 800, 0, width=1)

'''
canvas.create_rectangle(0, 0, 175, 120, outline="Black", fill="Grey")
canvas.create_rectangle(0, 120, 175, 240, outline="Black", fill="Grey")
canvas.create_rectangle(0, 240, 175, 360, outline="Black", fill="Grey")
canvas.create_rectangle(0, 360, 175, 480, outline="Black", fill="Grey")
canvas.create_rectangle(0, 480, 175, 600, outline="Black", fill="Grey")
'''

label1 = Label(text='Automatische Bediening:', font=("Courier", 10), height = 3, width = 22, bg="grey")
canvas.create_window(300, 75,  window=label1)


label2 = Label(text='Min. uitrolstant:', font=("Courier", 8), height = 3, width = 22, bg="grey")
canvas.create_window(270, 140,  window=label2)
slider = Scale(root, from_= 0, to=100, orient=HORIZONTAL, bg="grey", borderwidth ="0", highlightthickness = "0")
canvas.create_window(400, 130, window=slider)

label3 = Label(text='Max. uitrolstant:', font=("Courier", 8), height = 3, width = 22, bg="grey")
canvas.create_window(270, 190,  window=label3)
slider = Scale(root, from_= 0, to=100, orient=HORIZONTAL, bg="grey", borderwidth ="0", highlightthickness = "0")
canvas.create_window(400, 180, window=slider)

label4 = Label(text='Drempelwaarde Temp:', font=("Courier", 8), height = 3, width = 22, bg="grey")
canvas.create_window(270, 240,  window=label4)
slider = Scale(root, from_= 0, to=100, orient=HORIZONTAL, bg="grey", borderwidth ="0", highlightthickness = "0")
canvas.create_window(400, 230, window=slider)

button = Checkbutton(root, text="Handmatige bediening:", font=("Courier", 10), bg = "grey", borderwidth ="0", highlightthickness = "0")
canvas.create_window(300, 350,  window=button)

label3 = Label(text='Uitrolstant:', font=("Courier", 8), height = 3, width = 22, bg="grey")
canvas.create_window(270, 400,  window=label3)
slider = Scale(root, from_= 0, to=100, orient=HORIZONTAL, bg="grey", borderwidth ="0", highlightthickness = "0")
canvas.create_window(400, 390, window=slider)



button1 = Button(text='Controller 1', height = 3, width = 20)
canvas.create_window(90, 100,  window=button1)
button2 = Button(text='Controller 2', height = 3, width = 20)
canvas.create_window(90, 180,  window=button2)
button3 = Button(text='Controller 3', height = 3, width = 20)
canvas.create_window(90, 260,  window=button3)
button4 = Button(text='Controller 4', height = 3, width = 20)
canvas.create_window(90, 340,  window=button4)
button5 = Button(text='Controller 5', height = 3, width = 20)
canvas.create_window(90, 420,  window=button5)

root.mainloop()