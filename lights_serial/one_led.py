import serial
from serial_consts import *

s = serial.Serial(SERIAL_ADDR, BAUD, timeout=1)
try:
    s.open()
except:
    s.close()
    s.open()

data = [0] * TOTAL_BYTES
s.write(''.join(chr(d) for d in data))
s.flush()

while True:
    # capture input
    led = input('Toggle LED # (of %d): ' % TOTAL_LEDS)

    try:
        idx = int(led)
        for i in range(idx * 3, idx * 3 + 3):
            data[i] = 0 if data[i] == 100 else 100
        s.write(''.join(chr(d) for d in data))
        s.flush()
    except Exception as e:
        print e

