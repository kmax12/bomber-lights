import copy
import curses
import random
import serial
import sys
import time
import traceback

LEDS_PER_STRIP = 167
NUM_STRIPS = 8
TOTAL_LEDS = (LEDS_PER_STRIP * NUM_STRIPS)
BAUD = 115200
FRAMERATE = 30

def zip_sum(l1, l2):
    assert(len(l1) == len(l2))
    for i in range(len(l1)):
        l1[i] += l2[i]

class PulseController(object):
    def __init__(self, pad_row):
        self.key_row = '12345'
        self.key_map = {ord(c): i * TOTAL_LEDS / len(self.key_row) for i, c in
                        enumerate(list(self.key_row))}

        # each pulse is represented as an intensity and a color
        self.pulses = [[0, [0,0,0]]] * len(self.key_row)
        self.pulse_rad = TOTAL_LEDS / (len(self.key_row) * 2)
        self.pad_row = pad_row

    def process_input(self, key):
        # Check for inputs: the numbers 1-9 control pulses of brightness
        if key in self.key_map:
            idx = self.key_row.index(chr(key))
            color = [random.random() for j in range(3)]
            self.pulses[idx] = [0.99, color]

    def update_leds(self, leds, delta_t):
        # update brightnesses for each pulse
        for idx, pulse in enumerate(self.pulses):
            fade = (1.0 / self.pulse_rad) * pulse[0]
            color = pulse[1]
            pulse[0] *= 0.9     # exponential pulse intensity fade
            led_idx = self.key_map[ord(self.key_row[idx])]
            for i in range(self.pulse_rad):
                li = led_idx + i - self.pulse_rad
                if li <= 0:
                    break
                zip_sum(leds[li], [elt * fade * i for elt in color])

            for i in range(self.pulse_rad, 0, -1):
                li = led_idx + self.pulse_rad - i
                if li >= TOTAL_LEDS:
                    break
                zip_sum(leds[li], [elt * fade * i for elt in color])

    def update_pad(self, pad):
        for idx, pulse in enumerate(self.pulses):
            try:
                pad.addch(self.pad_row, idx, ord(str(int(pulse[0] * 10))))
            except:
                pass


class Controller(object):
    def __init__(self, pad_row):
        self.key_row = '12345'
        self.key_map = {ord(c): i * TOTAL_LEDS / len(self.key_row) for i, c in
                        enumerate(list(self.key_row))}

        # each pulse is represented as an intensity and a color
        self.pulses = [[0, [0,0,0]]] * len(self.key_row)
        self.pulse_rad = TOTAL_LEDS / (len(self.key_row) * 2)
        self.pad_row = pad_row

    def process_input(self, key):
        # Check for inputs: the numbers 1-9 control pulses of brightness
        if key in self.key_map:
            idx = self.key_row.index(chr(key))
            color = [random.random() for j in range(3)]
            self.pulses[idx] = [0.99, color]

    def update_leds(self, leds, delta_t):
        # update brightnesses for each pulse
        for idx, pulse in enumerate(self.pulses):
            fade = (1.0 / self.pulse_rad) * pulse[0]
            color = pulse[1]
            pulse[0] *= 0.9     # exponential pulse intensity fade
            led_idx = self.key_map[ord(self.key_row[idx])]
            for i in range(self.pulse_rad):
                li = led_idx + i - self.pulse_rad
                if li <= 0:
                    break
                zip_sum(leds[li], [elt * fade * i for elt in color])

            for i in range(self.pulse_rad, 0, -1):
                li = led_idx + self.pulse_rad - i
                if li >= TOTAL_LEDS:
                    break
                zip_sum(leds[li], [elt * fade * i for elt in color])

    def update_pad(self, pad):
        for idx, pulse in enumerate(self.pulses):
            try:
                pad.addch(self.pad_row, idx, ord(str(int(pulse[0] * 10))))
            except:
                pass


def quit_curses(stdscr):
    curses.nocbreak()
    stdscr.keypad(0)
    curses.echo()
    curses.endwin()


def main(stdscr):
    blank_leds = [[0, 0, 0] for i in range(TOTAL_LEDS)]
    curses.noecho()
    curses.cbreak()
    stdscr.nodelay(1)

    # Open serial port connection
    s = serial.Serial('/dev/ttyACM0', BAUD, timeout=1)
    try:
        s.open()
    except:
        s.close()
        s.open()

    # Initialize light controllers
    controllers = []
    controllers.append(PulseController(0))

    # initialize pad with (rows, columns)
    rows = len(controllers)
    columns = max(len(ct.key_row) for ct in controllers)
    pad = curses.newpad(rows, columns)

    while True:
        # process input
        key = 0
        while key > -1:
            # grab input key code
            key = stdscr.getch()

            # <Esc> quits the program
            if key == 27:
                print "Exiting..."
                quit_curses(stdscr)
                exit(0)

            # Pass other inputs on to controllers
            for ctrl in controllers:
                ctrl.process_input(key)

        leds = copy.deepcopy(blank_leds)
        for ctrl in controllers:
            ctrl.update_leds(leds, 1.0 / FRAMERATE)

        for ctrl in controllers:
            ctrl.update_pad(pad)

        # generate data string to send over serial
        data = ''
        for color in leds:
            for i in range(len(color)):
                data += chr(int(color[i] * 256))

        # send data and flush
        s.write(data)
        s.flush()

        # update ui
        pad.refresh(0, 0, 0, 0, rows, columns)

        time.sleep(1.0 / FRAMERATE)

if __name__ == '__main__':
    stdscr = curses.initscr()
    try:
        main(stdscr)
    except Exception as e:
        # die with dignity
        quit_curses(stdscr)
        traceback.print_exc()
        print repr(e)
        exit(0)
