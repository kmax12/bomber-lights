import serial
import time
import random
import curses

LEDS_PER_STRIP = 167
NUM_STRIPS = 8
TOTAL_LEDS = LEDS_PER_STRIP * NUM_STRIPS
TOTAL_BYTES = TOTAL_LEDS * 3
BAUD = 115200

stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.nodelay(1)

pad = curses.newpad(1, 1)
pulse = False

s = serial.Serial('/dev/ttyACM0', BAUD, timeout=1)
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
