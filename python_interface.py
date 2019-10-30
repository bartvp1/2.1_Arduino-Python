from tkinter import *
import tkinter.ttk as ttk
from random import randint
import time

root = Tk()
root.title('Interface')
root.resizable(False, False)
root.geometry("800x450")

main_color = '#3c3f41'

left_canvas = Canvas(root, width=200, height=430, bg='#313335', highlightthickness=0)
main_canvas = Canvas(root, width=600, height=430, bg=main_color, highlightthickness=0)
status_bar = Canvas(root, width=800, height=20, bg='#2b2b2b', highlightthickness=0)

status_bar.place(x=0, y=430)
left_canvas.place(x=0, y=0)
main_canvas.place(x=200, y=0)

configuration = [
    {'manual': False, 'manual_extension': 50, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 1
    {'manual': False, 'manual_extension': 50, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 2
    {'manual': False, 'manual_extension': 50, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 3
    {'manual': False, 'manual_extension': 50, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 4
    {'manual': False, 'manual_extension': 50, 'min_extension': 0, 'max_extension': 100, 'threshold': 0}]  # controller 5

current_controller = -1

def settings_panel(index):
    global current_controller
    main_canvas.delete("all")
    draw_navigation()
    current_controller = index
    if current_controller != -1:
        title = "Controller configuration "+str(current_controller+1)
        label1 = Label(text=title, font='Courier 12 bold', bg=main_color, fg='#ffffff')
        main_canvas.create_window(150, 50, window=label1)

        min_label = Label(text='Min. uitrolstand (%):', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        min_slider = Scale(root, from_=0, to=100, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
        main_canvas.create_window(100, 100, window=min_label)
        main_canvas.create_window(240, 90, window=min_slider)

        max_label = Label(text='Max. uitrolstand (%):', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        max_slider = Scale(root, from_=0, to=100, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
        main_canvas.create_window(100, 140, window=max_label)
        main_canvas.create_window(240, 130, window=max_slider)

        drempel_label = Label(text='Drempelwaarde °C:', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        drempel_slider = Scale(root, from_=0, to=100, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
        main_canvas.create_window(100, 180, window=drempel_label)
        main_canvas.create_window(240, 170, window=drempel_slider)

        manual_label = Label(text='Manual control', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        manual_button = Checkbutton(root, selectcolor=main_color, bg=main_color, highlightthickness=0, activebackground=main_color, activeforeground='#ffffff', fg='#ffffff')
        main_canvas.create_window(100, 240, window=manual_label)
        main_canvas.create_window(200, 240, window=manual_button)

        uitrol_label = Label(text='Uitrolstand:', font='Courier 8', height=3, width=22, fg='#ffffff', bg=main_color, anchor='w')
        uitrol_slider = Scale(root, from_=0, to=100, orient=HORIZONTAL, fg='#ffffff', bg=main_color, borderwidth="0", highlightthickness=0)
        main_canvas.create_window(100, 280, window=uitrol_label)
        main_canvas.create_window(240, 270, window=uitrol_slider)

        apply_btn = Button(text='Apply', width=7, command=lambda: apply_settings())
        main_canvas.create_window(260, 320, window=apply_btn)

def draw_graph():
    main_canvas.create_line(600, 550, 1150, 550, width=2)  # x-axis
    main_canvas.create_line(600, 550, 600, 50, width=2)  # y-axis
    main_canvas.create_text(560, 520, angle="90", text="Value")
    main_canvas.create_text(620, 580, text="Time")

    # x-axis
    for i in range(11):
        x = 600 + (i * 50)
        main_canvas.create_line(x, 550, x, 50, width=1, dash=(2, 5))
        main_canvas.create_text(x, 550, text='%d' % (10 * i), anchor=N)

    # y-axis
    for i in range(11):
        y = 550 - (i * 50)
        main_canvas.create_line(600, y, 1150, y, width=1, dash=(2, 5))
        main_canvas.create_text(590, y, text='%d' % (10 * i), anchor=E)

def draw_navigation():
    controller_buttons = [
        Button(text='Controller 1', height=2, width=30, command=lambda: settings_panel(0)),
        Button(text='Controller 2', height=2, width=30, command=lambda: settings_panel(1)),
        Button(text='Controller 3', height=2, width=30, command=lambda: settings_panel(2)),
        Button(text='Controller 4', height=2, width=30, command=lambda: settings_panel(3)),
        Button(text='Controller 5', height=2, width=30, command=lambda: settings_panel(4))]

    offset = 60
    for button in controller_buttons:
        left_canvas.create_window(90, offset, window=button)
        offset += 45

def apply_settings():
    #todo: send data to UNO
    print('x')

draw_navigation()
root.mainloop()
