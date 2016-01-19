import jack
import threading

client = jack.Client('FloungeClient')
inp = client.inports.register('input')
event = threading.Event()
num_bufs = 0

@client.set_process_callback
def process(frames):
    buf = client.inports[0].get_buffer()

with client:
    # When entering this with-statement, client.activate() is called.
    # This tells the JACK server that we are ready to roll.
    # Our process() callback will start running now.

    # Connect the ports.  You can't do this before the client is activated,
    # because we can't make connections to clients that aren't running.
    # Note the confusing (but necessary) orientation of the driver backend
    # ports: playback ports are "input" to the backend, and capture ports
    # are "output" from it.

    capture = client.get_ports(is_physical=True, is_output=True)
    if not capture:
        raise RuntimeError("No physical capture ports")

    for src, dest in zip(capture, client.inports):
        client.connect(src, dest)

    playback = client.get_ports(is_physical=True, is_input=True)
    if not playback:
        raise RuntimeError("No physical playback ports")

    for src, dest in zip(client.outports, playback):
        client.connect(src, dest)

    print("Press Ctrl+C to stop")
    try:
        event.wait()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
