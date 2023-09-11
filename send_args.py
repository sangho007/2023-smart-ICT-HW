import socket
import argparse

# 명령줄 인자 처리
parser = argparse.ArgumentParser(description='Send a message to the server.')
parser.add_argument('message', type=str, help='The message to send')
args = parser.parse_args()

# 호스트와 포트 번호 설정 (수신 쪽과 동일해야 함)
host = 'localhost'  # 루프백 인터페이스 주소(자기 자신)
port = 12345       # 사용할 포트 번호

# 서버에 연결하기 위한 소켓 생성 및 연결 요청 보내기
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((host, port))
print('서버에 연결되었습니다.')

message = args.message  # 명령줄에서 받은 메시지

# 메시지 전송
client_socket.sendall(message.encode())

# ACK 대기 시간 설정 (초 단위)
timeout_seconds = 5

# ACK 대기
client_socket.settimeout(timeout_seconds)

try:
    received_data = client_socket.recv(1024).decode()
    print('받은 메시지:', received_data)

    # ACK 수신 성공    
except socket.timeout:
    print('ACK 대기 시간이 초과되었습니다. 재전송합니다.')

    # 메시지 재전송
    client_socket.sendall(message.encode())
    
    try:
        received_data = client_socket.recv(1024).decode()
        print('받은 메시지:', received_data)

        # ACK 수신 성공
        
            
    except socket.timeout:
        print('ACK 대기 시간이 다시 초과되었습니다. 전송 실패')

# 소켓 닫기
client_socket.close()
