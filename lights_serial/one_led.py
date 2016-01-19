import serial

LEDS_PER_STRIP = 167
LEDS_LAST_STRIP = 149
NUM_STRIPS = 8
TOTAL_LEDS = LEDS_PER_STRIP * (NUM_STRIPS-1) + LEDS_LAST_STRIP
TOTAL_BYTES = TOTAL_LEDS * 3
BAUD = 115200

s = serial.Serial('/dev/ttyACM0', BAUD, timeout=1)
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

