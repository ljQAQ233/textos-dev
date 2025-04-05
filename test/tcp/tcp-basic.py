import socket

HOST = '0.0.0.0'
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening on {PORT}...")

    while True:
        conn, addr = s.accept()
        print(f"Connection from {addr}")
        conn.close();

