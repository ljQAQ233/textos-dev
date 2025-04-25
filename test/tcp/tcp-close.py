import socket

HOST = '0.0.0.0'
REMOTE = '192.168.2.2'
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((REMOTE, PORT))
    s.send(
        b"GET /config.ini HTTP/1.1\r\n"
        b"Host: 192.168.2.2\r\n"
        b"User-Agent: curl/0.1\r\n"
        b"\r\n")
    
    buf = b''

    while True:
        data = s.recv(1024)
        if not data:
            break
        buf += data

    print(buf)
    while True:
        []
