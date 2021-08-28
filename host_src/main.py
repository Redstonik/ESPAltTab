import sys
import socket
from pynput.keyboard import Key, Controller
import time

dev_port = 9001

keyboard = Controller()
def supCtrlRight():
    print("{} Fired!".format(time.strftime("%H:%M:%S", time.localtime())))
    with keyboard.pressed(Key.cmd):
        with keyboard.pressed(Key.ctrl):
            keyboard.press(Key.right)
            keyboard.release(Key.right)

def altTab():
    print("{} Fired!".format(time.strftime("%H:%M:%S", time.localtime())))
    with keyboard.pressed(Key.alt):
        keyboard.press(Key.tab)
        keyboard.release(Key.tab)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.settimeout(2.5)

print("Searching for devices:")
sock.sendto("S".encode(), ('<broadcast>', dev_port))
devices = []
while True:
    try:
        data, addr = sock.recvfrom(64)
        devices.append((data.decode(), addr))
        print(len(devices),devices[-1])
    except socket.timeout:
        break

dev = input("Select device:")
print("Connecting to {}".format(dev[0]), end="")
if dev[0] == "r":
    dev = devices[int(dev[1:]) - 1]
    sock.sendto("CR".encode(), dev[1])
    afunc = lambda *args: None
else:
    dev = devices[int(dev) - 1]
    sock.sendto("CA".encode(), dev[1])
    afunc = altTab

while True:
    try:
        data, addr = sock.recvfrom(8)
        if data == b"K":
            print(" Connected!")
            last = time.monotonic()
        elif data == b"F":
            print(" Failed: device max connections reached")
            sys.exit()
        break
    except socket.timeout:
        print(".", end="")

print("Press Ctrl+C to exit")
while True:
    try:
        data, addr = sock.recvfrom(8)
        if data == b"\01":
            if(time.monotonic() - 1 > last):
                afunc()
                last = time.monotonic()
        elif data[0] == ord("R"):
            print(int.from_bytes(data[1:], 'little'))
    except socket.timeout:
        afunc()
        print("Lost connection")
        break
    except KeyboardInterrupt:
        print("Closing connection...")
        try:
            sock.sendto("D".encode(), dev[1])
            data, addr = sock.recvfrom(8)
            if data == b"B":
                print("Disconnected")
        except socket.timeout:
            print("Disconnect confirmation not recived")
        break
