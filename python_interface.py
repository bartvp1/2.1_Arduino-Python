from tkinter import *

window = Tk()
window.title("Control Panel")

root = Frame(window)
root.pack()
canvas = Canvas(root, width=1368, height=768)
canvas.pack()

frame = Frame(window)
frame.pack()

canvas.create_line(50, 25, 200, 25, width=10)
button = Button(frame, text="button", width=10, height=2)

# , command=lambda: add_line())
button.pack()#place(x=0, y=0)



window.mainloop()
