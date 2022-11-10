import socket

SERVER  = "192.168.1.195"
PORT    = 7
ADDR    = (SERVER, PORT)
FORMAT  = "utf-8"

mcu = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
mcu.connect(ADDR)

message = "CONFIG".encode(FORMAT)
mcu.sendto(message, ADDR)
while True:
    print(mcu.recv(1024).decode(FORMAT))
