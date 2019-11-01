from tkinter import *
import tkinter.ttk as ttk
from random import randint
from pprint import pprint
import serial.tools.list_ports

root = Tk()
root.title('Centrale interface')
root.resizable(False, False)
root.geometry("800x450")

main_color = '#3c3f41'
settings_panel_open = False
current_controller = -1
arduinos = {}

left_canvas = Canvas(root, width=200, height=430, bg='#313335', highlightthickness=0)
main_canvas = Canvas(root, width=600, height=430, bg=main_color, highlightthickness=0)
status_bar = Canvas(root, width=800, height=20, bg='#2b2b2b', highlightthickness=0)

status_bar.place(x=0, y=430)
left_canvas.place(x=0, y=0)
main_canvas.place(x=200, y=0)

configuration = [
    {'manual': False, 'manual_extension': 0, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 1
    {'manual': False, 'manual_extension': 0, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 2
    {'manual': False, 'manual_extension': 0, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 3
    {'manual': False, 'manual_extension': 0, 'min_extension': 0, 'max_extension': 100, 'threshold': 0},  # controller 4
    {'manual': False, 'manual_extension': 0, 'min_extension': 0, 'max_extension': 100, 'threshold': 0}]  # controller 5

def settings_panel(index):
    global current_controller, settings_panel_open
    settings_panel_open = not settings_panel_open
    current_controller = index
    sensor = arduinos.get(list(arduinos.keys())[current_controller])
    title = Label(text=sensor+" configuration", font='Courier 12 bold', bg=main_color, fg='#ffffff', anchor=W)
    min_slider = Scale(root, from_=0, to=100, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
    max_slider = Scale(root, from_=min_slider.get(), to=100, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
    drempel_slider = Scale()
    uitrol_slider = Scale(root, from_=-30, to=50, resolution=5, orient=HORIZONTAL, fg='#ffffff', bg=main_color, borderwidth="0", highlightthickness=0)

    drempel_slider.set(configuration[current_controller]['threshold'])
    max_slider.set(configuration[current_controller]['max_extension'])
    min_slider.set(configuration[current_controller]['min_extension'])
    uitrol_slider.set(configuration[current_controller]['manual_extension'])
    manual_var = BooleanVar()
    manual_button = Checkbutton(root, selectcolor=main_color, bg=main_color, highlightthickness=0, activebackground=main_color, activeforeground='#ffffff', fg='#ffffff', variable=manual_var)

    if configuration[current_controller]['manual']:
        manual_button.select()
    else:
        manual_button.deselect()

    min_label = Label(text='Min. uitrolstand (cm):', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
    max_label = Label(text='Max. uitrolstand (cm):', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
    drempel_label = Label()
    manual_warning = Label(text="Manual control will override automatic behaviour", font='Courier 8', bg=main_color, fg='#ffffff')
    manual_label = Label(text='Manual control', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
    uitrol_label = Label(text='Uitrolstand (cm):', font='Courier 8', height=3, width=22, fg='#ffffff', bg=main_color, anchor='w')
    apply_btn = Button(text='Apply', width=7, command=lambda: apply_settings([min_slider.get(), max_slider.get(), drempel_slider.get(), manual_var.get(), uitrol_slider.get()]))

    if sensor == 'Lichtsensor':
        drempel_label = Label(text='Drempelwaarde (lux):', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='e')
        drempel_slider = Scale(root, from_=10000, to=100000, resolution=10000, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)

    if sensor == 'Temperatuursensor':
        drempel_label = Label(text='Drempelwaarde (°C):', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        drempel_slider = Scale(root, from_=-30, to=50, resolution=2, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)

    main_canvas.delete("all")
    main_canvas.create_window(170, 50, window=title)
    main_canvas.create_window(100, 100, window=min_label)
    main_canvas.create_window(240, 90, window=min_slider)
    main_canvas.create_window(100, 140, window=max_label)
    main_canvas.create_window(240, 130, window=max_slider)
    main_canvas.create_window(100, 180, window=drempel_label)
    main_canvas.create_window(240, 170, window=drempel_slider)
    main_canvas.create_window(190, 240, window=manual_warning)
    main_canvas.create_window(100, 280, window=manual_label)
    main_canvas.create_window(200, 280, window=manual_button)
    main_canvas.create_window(100, 320, window=uitrol_label)
    main_canvas.create_window(240, 310, window=uitrol_slider)
    main_canvas.create_window(260, 360, window=apply_btn)

def draw_graph():
    posx, posy, width, height = 400, 120, 180, 80
    main_canvas.create_line(posx, posy+height, posx+width, posy+height, width=1)        # x-axis
    main_canvas.create_line(posx, posy+height, posx, posy, width=1)                     # y-axis
    main_canvas.create_text(posx-15, posy-15, text="°C", font='Courier 8')              # y-unit
    main_canvas.create_text(posx+30, posy+height+25, text="Time", font='Courier 8')     # x-unit

    # x-axis
    for i in range(0, 4):
        x = 400 + (i * 60)
        main_canvas.create_line(x, 200, x, 120, width=1, dash=(2, 5))
        main_canvas.create_text(x, 210, text='%d' % (60 * i), font='Courier 6', anchor=N)

    # y-axis
    for i in range(0, 5):
        y = 200 - (i * 20)
        main_canvas.create_line(400, y, 580, y, width=1, dash=(2, 5))
        main_canvas.create_text(395, y, text='%d' % (20 * i), font='Courier 6', anchor=E)

def refresh_ports():
    return [p.device for p in serial.tools.list_ports.comports() if p.pid == 67]

def refresh_arduinos():
    # map connected arduino in dictionary (key=port, value=sensor type)
    for port in refresh_ports():
        global ser
        try:
            ser = serial.Serial(port, 9600)
            while ser.read():
                line = str(ser.readline().lower().decode(encoding='UTF-8'))
                if line.find('lichtsensor') != -1 and arduinos.get(port) is None:
                    arduinos.update({port: 'Lichtsensor'})
                if line.find('temperatuur') != -1 and arduinos.get(port) is None:
                    arduinos.update({port: 'Temperatuursensor'})
                break
            ser.close()
        except serial.serialutil.SerialException:
            ser.close()

def draw_navigation():
    left_canvas.delete("all")
    offset = 60
    if len(refresh_ports()) == 0:
        main_canvas.delete("all")
        title = Label(text='Please connect a controller', font='Courier 12 bold', bg=main_color, fg='#ffffff')
        main_canvas.create_window(150, 50, window=title)
    else:
        for i in range(len(arduinos.items())):
            button = Button(text=arduinos.get(list(arduinos.keys())[i]), fg='white', bg='black', activebackground='black', activeforeground='white', height=2, width=30, command=lambda: settings_panel(i)),
            left_canvas.create_window(90, offset, window=button)
            offset += 45
        if not settings_panel_open:
            main_canvas.delete("all")
            title = Label(text='Please select a controller', font='Courier 12 bold', bg=main_color, fg='#ffffff')
            main_canvas.create_window(150, 50, window=title)

def apply_settings(obj):
    #todo: send data to UNO
    configuration[current_controller].update({'manual': obj[3], 'manual_extension': obj[4], 'min_extension': obj[0], 'max_extension': obj[1], 'threshold': obj[2]})
    pprint(configuration[current_controller])

def draw_status():
    status_bar.delete("all")
    from_ = 5
    for i in range(0, len(refresh_ports())):
        status_bar.create_oval(from_, 5, 15*(i+1), 15, fill='#26e300', outline="grey", width=1)
        from_ += 15
    for i in range(len(refresh_ports()), 6):
        status_bar.create_oval(from_, 5, 15*(i+1), 15, fill='grey', outline="grey", width=1)
        from_ += 15
    status_bar.create_text(100, 10, text=str(len(refresh_ports()))+'/5 controllers connected', fill='white', font='Courier 8', anchor=W)


def update_devices():
    global arduinos
    ports = refresh_ports()
    print(arduinos, len(ports), len(arduinos))
    if len(ports) != len(arduinos):

        if len(ports) > len(arduinos):
            print('device connected')

        elif len(ports) < len(arduinos):
            print('device disconnected')

        arduinos.clear()
        refresh_arduinos()
        status_bar.after(0, draw_status)
        left_canvas.after(0, draw_navigation)

    root.after(500, update_devices)

draw_status()
draw_navigation()
update_devices()
root.mainloop()
