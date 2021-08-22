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

dev = devices[int(input("Select device:")) - 1]
print("Connecting to {}".format(dev[0]), end="")
sock.sendto("C".encode(), dev[1])
while True:
    try:
        data, addr = sock.recvfrom(8)
        print(" Connected!")
        last = time.monotonic()
        break
    except socket.timeout:
        print(".", end="")


while True:
    try:
        data, addr = sock.recvfrom(8)
        if data == b"\01":
            if(time.monotonic() - 1 > last):
                altTab()
                last = time.monotonic()
    except socket.timeout:
        altTab()
        print("Lost connection")
        break