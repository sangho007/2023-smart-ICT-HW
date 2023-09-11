import serial
import time

import socket

ser = serial.Serial("/dev/cu.usbmodem84301", 9600, timeout=1)
time.sleep(2)


def send_command(command):
    ser.write((command + "\n").encode("utf-8"))
    time.sleep(0.05)  # Add delay
    response = ser.readline().decode("utf-8", errors="replace").strip()
    print(response)


def printText():
    print("\n")
    print("o : door open")
    print("c : door close\n")
    print("g : green LED.")
    print("r : red LED.\n")
    print("---------------------------\n")


printText()


host = "localhost"  # 루프백 인터페이스 주소(자기 자신)
port = 12345  # 사용할 포트 번호
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))

# 클라이언트의 연결 대기
server_socket.listen(1)
print("수신 대기 중...")

try:
    while True:
        # 클라이언트 연결 수락
        client_socket, addr = server_socket.accept()
        print("연결됨:", addr)

        while True:
            data = client_socket.recv(1024).decode()

            if not data:
                # 데이터가 없으면 해당 클라이언트와의 연결 종료하고 새로운 클라이언트 기다림.
                print("클라이언트와의 연결 종료.")
                break

            print("수신한 메시지:", data)

            if data == "o":
                send_command("open")
                print("open 전송 - 관제센터 사이니지 문 열어라.")
                time.sleep(0.05)
            elif data == "c":
                send_command("close")
                print("close 전송 - 관제센터 사이니지 문 닫아라.")
                time.sleep(0.05)
            elif data == "g":
                send_command("g")
                print("green 전송 - 관제센터 지진해일 소강")
                time.sleep(0.05)
            elif data == "r":
                send_command("r")
                print("red 전송 - 관제센터 지진해일 위험 경보 발생")
                time.sleep(0.05)

            # 받은 데이터를 그대로 클라이언트에게 전송 (에코)
            client_socket.sendall(data.encode())


except KeyboardInterrupt:
    pass

finally:
    server_socket.close()
