import socket

HOST = '0.0.0.0'
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    conn, add = s.accept()
    with conn:
        conn.send(b'test delay ack', 0)
        rx = conn.recv(1024)
        print(rx)
