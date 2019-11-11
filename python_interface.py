import tkinter
from tkinter import *
from functools import partial
import serial.tools.list_ports
import matplotlib.pyplot as plt

root = Tk()
root.title('Centrale interface')
root.resizable(False, False)
root.geometry("800x450")

main_color = '#3c3f41'
settings_panel_open = False
current_port = ""
arduinos = {}
serials = {}
reading = False

left_canvas = Canvas(root, width=200, height=430, bg='#313335', highlightthickness=0)
main_canvas = Canvas(root, width=600, height=430, bg=main_color, highlightthickness=0)
status_bar = Canvas(root, width=800, height=20, bg='#2b2b2b', highlightthickness=0)

configuration = {}

status_bar.place(x=0, y=430)
left_canvas.place(x=0, y=0)
main_canvas.place(x=200, y=0)


def settings_panel(port):
    # print(str(port), arduinos.get(port))
    global current_port, settings_panel_open
    settings_panel_open = not settings_panel_open
    current_port = port
    sensor = arduinos.get(current_port)
    title = Label(text=sensor+" configuration", font='Courier 12 bold', bg=main_color, fg='#ffffff', anchor=W)
    sv = StringVar(root, 100)
    maximale_uitrolstand_entry = Entry(root, textvariable=sv, width=4)
    min_slider = Scale(root, from_=0, to=100, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
    max_slider = Scale(root, from_=0, to=100, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)
    drempel_slider = Scale()
    uitrol_var = BooleanVar()
    uitrol_check = Checkbutton(root, selectcolor=main_color, bg=main_color, highlightthickness=0, activebackground=main_color, activeforeground='#ffffff', fg='#ffffff', variable=uitrol_var)
    manual_var = BooleanVar()
    manual_button = Checkbutton(root, selectcolor=main_color, bg=main_color, highlightthickness=0, activebackground=main_color, activeforeground='#ffffff', fg='#ffffff', variable=manual_var)

    maximale_uitrolstand_label = Label(text='Maximale hoogte (cm)', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor=W)
    min_label = Label(text='Min. uitrolstand (%)', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor=W)
    max_label = Label(text='Max. uitrolstand (%)', font=("Courier", 8), fg='#ffffff', height=3, width=22, bg=main_color, anchor=W)
    drempel_label = Label()
    manual_warning = Label(text="Manual control will override automatic behaviour", font='Courier 8', bg=main_color, fg='#ffffff')
    manual_label = Label(text='Manual control', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor=W)
    uitrol_label = Label(text='Uitrollen', font='Courier 8', height=3, width=22, fg='#ffffff', bg=main_color, anchor=W)
    apply_btn = Button(text='Apply', width=7, command=lambda: apply_settings([sv.get(), min_slider.get(), max_slider.get(), drempel_slider.get(), manual_var.get(), uitrol_var.get()]))

    if sensor == 'Lichtsensor':
        drempel_label = Label(text='Drempelwaarde (lux)', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor=W)
        drempel_slider = Scale(root, from_=0, to=300, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)

    if sensor == 'Temperatuursensor':
        drempel_label = Label(text='Drempelwaarde (°C)', font='Courier 8', fg='#ffffff', height=3, width=22, bg=main_color, anchor='w')
        drempel_slider = Scale(root, from_=-30, to=50, resolution=5, orient=HORIZONTAL, bg=main_color, fg='#ffffff', borderwidth="0", highlightthickness=0)

    if current_port in configuration:

        max_slider.set(configuration[current_port]['auto_max_extension'])
        min_slider.set(configuration[current_port]['auto_min_extension'])
        drempel_slider.set(configuration[current_port]['threshold'])

        if configuration[current_port]['manual_extension']:
            uitrol_check.select()
        else:
            uitrol_check.deselect()

        if configuration[current_port]['manual']:
            manual_button.select()
        else:
            manual_button.deselect()

    main_canvas.delete("all")
    #draw_graph()
    x1 = 15
    x2 = 200
    main_canvas.create_window(x1, 50, window=title, anchor=W)

    main_canvas.create_window(x1, 100, window=maximale_uitrolstand_label, anchor=W)
    main_canvas.create_window(x1, 140, window=min_label, anchor=W)
    main_canvas.create_window(x1, 180, window=max_label, anchor=W)
    main_canvas.create_window(x1, 220, window=drempel_label, anchor=W)
    main_canvas.create_window(x1, 280, window=manual_warning, anchor=W)
    main_canvas.create_window(x1, 320, window=manual_label, anchor=W)
    main_canvas.create_window(x1, 350, window=uitrol_label, anchor=W)

    main_canvas.create_window(x2, 100, window=maximale_uitrolstand_entry, anchor=W)
    main_canvas.create_window(x2, 130, window=min_slider, anchor=W)
    main_canvas.create_window(x2, 170, window=max_slider, anchor=W)
    main_canvas.create_window(x2, 210, window=drempel_slider, anchor=W)
    main_canvas.create_window(x2, 320, window=manual_button, anchor=W)
    main_canvas.create_window(x2, 350, window=uitrol_check, anchor=W)

    main_canvas.create_window(x1, 400, window=apply_btn, anchor=W)

def draw_graph():
    plt.plot('xlabel', 'ylabel', data=temperatuur)

def get_ports():
    return [p.device for p in serial.tools.list_ports.comports() if p.pid == 67]

def refresh_arduinos():
    arduinos.clear()
    # map connected arduino in dictionary (key=port, value=sensor type)
    for port in get_ports():
        ser = serial.Serial()
        try:
            ser = serial.Serial(port, 9600)
            serials.update({port: ser})
            line = str(ser.readline().decode())
            print(line)
            if line.find('licht') != -1:
                arduinos.update({port: 'Lichtsensor'})
            if line.find('temperatuur') != -1:
                arduinos.update({port: 'Temperatuursensor'})
        except serial.serialutil.SerialException:
            print("serial connection failed")
            ser.close()


licht = []
temperatuur = []
def read_serial():
    for port in arduinos:
        ser = serials.get(port)
        try:
            line = str(ser.readline().decode())
            while int(line.find("\n")) != -1:
                print("serial_receive: "+line)
                if line.find("l=") is not -1:
                    waarde = line[line.find("l=")+2:-1]
                    print("l:"+waarde)
                    licht.append(int(waarde))
                if line.find("t=") is not -1:
                    waarde = line[line.find("t=")+2:-1]
                    temperatuur.append(int(waarde))
                #print("licht: "+str(licht)+"\ntemperatuur: "+str(temperatuur)+"\n")
                break
        except:
            print("error decoding serial data")
    root.after(6000, read_serial)

def draw_navigation():
    global reading
    left_canvas.delete("all")
    offset = 60
    if len(get_ports()) == 0:
        main_canvas.delete("all")
        title = tkinter.Label(text='Please connect a controller', font='Courier 12 bold', bg=main_color, fg='#ffffff')
        main_canvas.create_window(150, 50, window=title)
    else:
        for i in arduinos.keys():
            left_canvas.create_window(90, offset, window=tkinter.Button(text=arduinos.get(i), fg='white', bg='black', activebackground='black', activeforeground='white', height=2, width=30, command=partial(settings_panel, i)))
            offset += 45
        if not settings_panel_open:
            main_canvas.delete("all")
            title = tkinter.Label(text='Please select a controller', font='Courier 12 bold', bg=main_color, fg='#ffffff')
            main_canvas.create_window(150, 50, window=title)
    if not reading:
        reading = True
        read_serial()

def apply_settings(obj):
    try:
        uitrol = int(obj[0])
    except:
        uitrol = 100
    configuration.update({current_port: {'max_extension': uitrol, 'auto_min_extension': obj[1], 'auto_max_extension': obj[2], 'threshold': obj[3], 'manual': obj[4], 'manual_extension': obj[5]}})

    #print(current_port, " -> ", str(configuration[current_port]).encode())
    serOutput = serials.get(current_port)
    serOutput.flushInput()
    serOutput.write(str(configuration[current_port]).encode())
    #serOutput.write(str("{setting1:100}").encode())

def draw_status():
    status_bar.delete("all")
    from_ = 5
    for i in range(0, len(arduinos)):
        status_bar.create_oval(from_, 5, 15*(i+1), 15, fill='#26e300', outline="grey", width=1)
        from_ += 15
    for i in range(len(arduinos), 6):
        status_bar.create_oval(from_, 5, 15*(i+1), 15, fill='grey', outline="grey", width=1)
        from_ += 15
    status_bar.create_text(100, 10, text=str(len(arduinos))+'/5 controllers connected', fill='white', font='Courier 8', anchor=W)

def update_devices():
    global arduinos
    ports = get_ports()
    #print(arduinos, {'ports': len(ports)}, {'arduinos': len(arduinos)})
    if len(ports) != len(arduinos):

        if len(ports) > len(arduinos):
            print(str(len(ports)-len(arduinos))+' device connected')
        elif len(ports) < len(arduinos):
            print(str(len(arduinos)-len(ports))+' device disconnected')
            if "Licht" not in arduinos.values():
                licht.clear()
            if "Temperatuur" not in arduinos.values():
                temperatuur.clear()

        refresh_arduinos()
        status_bar.after(0, draw_status)
        left_canvas.after(0, draw_navigation)
    root.after(500, update_devices)


draw_status()
draw_navigation()
update_devices()
root.mainloop()
