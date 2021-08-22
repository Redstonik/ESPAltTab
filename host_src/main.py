import socket
from pynput.keyboard import Key, Controller
import time


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
sock.settimeout(2.5)
sock.bind(("",4202))

print("Waiting for first packet", end="")
while True:
    try:
        data, addr = sock.recvfrom(8)
        break
    except socket.timeout:
        print(".",end="")


print("\nPacket from: {}".format(addr))
last = time.monotonic()
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