import serial
import time
import random
import curses
from serial_consts import *

FRAMERATE = 30

stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.nodelay(1)

pad = curses.newpad(1, 1)
pulse = False

s = serial.Serial(SERIAL_ADDR, BAUD, timeout=1)
try:
    s.open()
except:
    s.close()
    s.open()

while True:
    # capture input
    c = 0
    while c > -1:
        c = stdscr.getch()
        if c == 27:   # Esc
            curses.nocbreak()
            stdscr.keypad(0)
            curses.echo()
            curses.endwin()
            exit(0)

        if c == ord(' '):
            pulse = not pulse

    # form data string to send over serial
    data = chr(100) if pulse else chr(0)
    data *= TOTAL_BYTES

    # update ui
    pad.refresh(0, 0, 0, 0, 0, 1)
    s.write(data)
    s.flush()

    time.sleep(0.033)
