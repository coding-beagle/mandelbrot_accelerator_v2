import socket
import time


def test_server_reliability(test_host="192.168.1.3", test_port=7):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    for i in range(10):
        try:
            s = reconnect_if_needed(s, test_host, test_port)
            s.settimeout(2.0)

            test_msg = f"TEST{i}\n"
            s.send(test_msg.encode())

            response = s.recv(1024)
            print(
                f"Test {i}: Sent '{test_msg.strip()}', Got '{response.decode().strip()}'"
            )

            s.close()
            time.sleep(0.1)

        except Exception as e:
            print(f"Test {i} failed: {e}")


def reconnect_if_needed(socket_obj, host, port):
    try:
        # Test if connection is still alive
        socket_obj.settimeout(0.1)
        socket_obj.send(b"")  # Empty send to test connection
        return socket_obj
    except:
        # print(f"Reconnecting to {host}:{port}")
        socket_obj.close()
        new_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        new_socket.connect((host, port))
        return new_socket


test_server_reliability()
test_server_reliability("192.168.1.10", 7)
