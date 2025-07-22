import socket
import struct
from fixedpoint import FixedPoint
from enum import Enum
from typing import Union
from time import time_ns, time
import re
import numpy as np
from PIL import Image
import cv2 as cv
import keyboard as kb

# CONNECTION STUFF

# take the server name and port name
host = '192.168.1.10'
port = 7

# RENDERING SETTINGS

PIXELS_PER_LINE_LOW = 320
TOTAL_Y_LOW = 240

PIXELS_PER_LINE = 640
TOTAL_Y = 480

# create a socket at client side
# using TCP / IP protocol
s = socket.socket(socket.AF_INET,
                  socket.SOCK_STREAM)

s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)

# connect it to server and port
# number on local computer.
s.connect((host, port))


class MandelbrotStreamingClient:
    def __init__(self, socket_obj):
        self.sock = socket_obj
        self.buffer = b''

    def flush_socket_buffer(self):
        """Clear any remaining data in socket buffer"""
        self.sock.settimeout(0.1)
        try:
            while True:
                data = self.sock.recv(4096)
                if not data:
                    break
        except socket.timeout:
            pass
        finally:
            self.sock.settimeout(10.0)
            self.buffer = b''

    def receive_until_delimiter(self, delimiter=b'\n'):
        """Receive data until we find the delimiter"""
        while delimiter not in self.buffer:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError(
                    "Socket closed before receiving delimiter")
            self.buffer += chunk

        # Split at first delimiter
        line, self.buffer = self.buffer.split(delimiter, 1)
        return line

    def receive_exact(self, length):
        """Receive exactly 'length' bytes"""
        while len(self.buffer) < length:
            chunk = self.sock.recv(length - len(self.buffer))
            if not chunk:
                raise ConnectionError(
                    "Socket closed before receiving all data")
            self.buffer += chunk

        # Extract the required amount
        data = self.buffer[:length]
        self.buffer = self.buffer[length:]
        return data

    def start_streaming(self, low_res: bool = False):
        """Start the streaming process and return stream info"""
        self.flush_socket_buffer()
        if (low_res):
            self.sock.send("LOWRE".encode())
        else:
            self.sock.send("STREM".encode())

        # Wait for stream start confirmation
        response = self.receive_until_delimiter()
        response_str = response.decode()

        if not response_str.startswith("STREAM_START"):
            raise ValueError(f"Unexpected response: {response_str}")

        # Parse total lines from response: "STREAM_START:LINES:1080"
        match = re.search(r'LINES:(\d+)', response_str)
        if match:
            total_lines = int(match.group(1))
            print(f"Starting stream for {total_lines} lines")
            return total_lines
        else:
            raise ValueError("Could not parse line count from stream start")

    def receive_streaming_line(self):
        """Receive one line from the stream"""
        line_data = self.receive_until_delimiter()
        line_str = line_data.decode()

        if line_str == "STREAM_COMPLETE":
            return None, None  # End of stream

        # Parse line header: "LINE:0001:hexdata"
        if not line_str.startswith("LINE:"):
            raise ValueError(f"Unexpected line format: {line_str[:50]}...")

        # Extract line number and hex data
        parts = line_str.split(':', 2)
        if len(parts) != 3:
            raise ValueError(f"Invalid line format: {line_str[:50]}...")

        line_number = int(parts[1])
        hex_data = parts[2]

        # Convert hex string to byte array
        if len(hex_data) % 2 != 0:
            raise ValueError(f"Hex data length not even: {len(hex_data)}")

        # Convert hex pairs to integers
        int_array = [int(hex_data[i:i+2], 16)
                     for i in range(0, len(hex_data), 2)]

        return line_number, np.array(int_array, dtype=np.uint8)

    def get_full_mandelbrot_stream(self, low_res: bool = False):
        """Get the complete Mandelbrot set via streaming"""
        total_lines = self.start_streaming(low_res=low_res)

        # Initialize the full image array
        line_length = None
        full_image = None
        received_lines = 0

        print("Receiving streamed data...")
        start_time = time()

        while True:
            line_number, line_data = self.receive_streaming_line()

            if line_number is None:  # Stream complete
                break

            # Initialize full image array on first line
            if full_image is None:
                line_length = len(line_data)
                full_image = np.zeros(
                    (total_lines, line_length), dtype=np.uint8)
                print(f"Image dimensions: {total_lines} x {line_length}")

            # Store the line data
            if line_number < total_lines:
                full_image[line_number] = line_data
                received_lines += 1

                if received_lines % 100 == 0:
                    elapsed = time() - start_time
                    progress = (received_lines / total_lines) * 100
                    print(
                        f"Progress: {progress:.1f}% ({received_lines}/{total_lines}) - {elapsed:.2f}s")

        elapsed = time() - start_time
        print(
            f"Stream complete! Received {received_lines} lines in {elapsed:.2f}s")

        return full_image


class NamedRegisters(Enum):
    CURRENT_X = "CURRX"
    CURRENT_Y = "CURRY"
    STEP_X = 'XSTEP'
    STEP_Y = 'YSTEP'
    TOP_LEFT_X = 'X_TOP'
    TOP_LEFT_Y = 'Y_TOP'
    Z_RE = 'ZREAL'
    Z_IM = 'ZIMAG'


def flush_socket_buffer():
    """Clear any remaining data in the socket buffer"""
    s.settimeout(0.1)  # Short timeout
    try:
        while True:
            data = s.recv(1024)
            if not data:
                break
            # print(f"Flushed: {data}")
    except socket.timeout:
        pass  # Expected when buffer is empty
    finally:
        s.settimeout(None)  # Reset to blocking


def check_reg(reg: str):
    valid_regs: list[str] = [e.value for e in NamedRegisters]
    if reg not in valid_regs:
        raise ValueError(
            f"Invalid register to send to, valid registers are {valid_regs}")


def custom_decode(bytes_val: bytes):
    return str(bytes_val).replace("\\x", "").replace("'", "").strip('b').upper()


def send_float(value: Union[float, str], reg: str):
    check_reg(reg)

    data_bytes = FixedPoint(
        value,
        signed=True,
        m=12,
        n=52,
        str_base=16,
    )

    data_bytes = bytes(struct.pack(">Q", data_bytes.bits))

    message = reg.encode() + " ".encode() + data_bytes + \
        chr(4).encode() + chr(4).encode()

    s.send(message)


def fetch_float(reg: str, ret_hex: bool = False):
    check_reg(reg)

    flush_socket_buffer()

    print(f"Fetching reg {reg}")

    message = reg.encode()
    s.send(message)

    s.settimeout(2.0)
    msg = s.recv(32)

    # print(f"Raw Resp = {msg}")
    if (ret_hex):
        return custom_decode(msg)

    return float(
        FixedPoint(
            "0x" + (custom_decode(msg)),
            signed=True,
            m=12,
            n=52,
        )
    )


# Updated usage functions for backward compatibility
def receive_exact(sock, length):
    """Legacy function - kept for compatibility"""
    client = MandelbrotStreamingClient(sock)
    return client.receive_exact(length)


def get_line_streaming(sock, low_res: bool = False):
    """New streaming version of get_line"""
    client = MandelbrotStreamingClient(sock)

    if (low_res):
        return client.get_full_mandelbrot_stream(True)
    else:
        return client.get_full_mandelbrot_stream()


def reset():
    flush_socket_buffer()

    s.send("RESET".encode())

    # return


def calculate():
    flush_socket_buffer()
    print("Starting full res calculation!")
    s.send("CALCE".encode())
    time = time_ns()

    s.settimeout(100.0)
    msg = s.recv(32)

    time2 = time_ns()
    print(msg.decode())
    print(f"Took {(time2 - time) / 10 ** 9} seconds to calculate!")


def send_defaults():
    send_float(-2, NamedRegisters.TOP_LEFT_X.value)
    print(fetch_float(NamedRegisters.TOP_LEFT_X.value))
    send_float(1, NamedRegisters.TOP_LEFT_Y.value)
    print(fetch_float(NamedRegisters.TOP_LEFT_Y.value))
    send_float(-3/PIXELS_PER_LINE,
               NamedRegisters.STEP_X.value)
    print(fetch_float(NamedRegisters.STEP_X.value))
    send_float(2/TOTAL_Y, NamedRegisters.STEP_Y.value)
    print(fetch_float(NamedRegisters.STEP_Y.value))
    send_float(0, NamedRegisters.CURRENT_X.value)
    send_float(0, NamedRegisters.CURRENT_Y.value)


send_defaults()

DEFAULT_CURRENT_LEFT = -2
DEFAULT_CURRENT_UP = 1
DEFALT_CURRENT_ZOOM = 1
DEFAULT_XSTEP = -3/PIXELS_PER_LINE_LOW
DEFAULT_YSTEP = 2/TOTAL_Y_LOW
DEFAULT_STEPPING = 0.1

data = np.zeros([TOTAL_Y, PIXELS_PER_LINE], np.uint8)

current_left = DEFAULT_CURRENT_LEFT
current_up = DEFAULT_CURRENT_UP
current_zoom = DEFALT_CURRENT_ZOOM
current_xstep = DEFAULT_XSTEP
current_ystep = DEFAULT_YSTEP
stepping_factor = DEFAULT_STEPPING


def increase_stepping_factor():
    global stepping_factor
    stepping_factor *= 1.1
    print(f"New stepping factor = {stepping_factor}")


def decrease_stepping_factor():
    global stepping_factor
    stepping_factor *= 0.9
    print(f"New stepping factor = {stepping_factor}")


def calc_and_redraw():
    global data
    send_float(0, NamedRegisters.CURRENT_X.value)
    send_float(0, NamedRegisters.CURRENT_Y.value)
    # calculate(True)
    # data_low = get_line_streaming(s, True)
    # data = data_low
    # data = cv.resize(data_low, (PIXELS_PER_LINE, TOTAL_Y))
    # data = data_low

    calculate()
    new_data = get_line_streaming(s)
    data = np.zeros([TOTAL_Y, PIXELS_PER_LINE], np.uint8)
    data = new_data


def pan_left():
    global current_left, stepping_factor
    current_left -= stepping_factor
    send_float(current_left, NamedRegisters.TOP_LEFT_X.value)
    print(
        f"fetched left value = {fetch_float(NamedRegisters.TOP_LEFT_X.value)}")
    calc_and_redraw()


def pan_right():
    global current_left, stepping_factor
    current_left += stepping_factor
    send_float(current_left, NamedRegisters.TOP_LEFT_X.value)
    print(
        f"fetched left value = {fetch_float(NamedRegisters.TOP_LEFT_X.value)}")
    calc_and_redraw()


def pan_up():
    global current_up, stepping_factor
    current_up += stepping_factor
    send_float(current_up, NamedRegisters.TOP_LEFT_Y.value)
    print(
        f"fetched up value = {fetch_float(NamedRegisters.TOP_LEFT_Y.value)}")
    calc_and_redraw()


def pan_down():
    global current_up, stepping_factor
    current_up -= stepping_factor
    send_float(current_up, NamedRegisters.TOP_LEFT_Y.value)
    print(
        f"fetched up value = {fetch_float(NamedRegisters.TOP_LEFT_Y.value)}")
    calc_and_redraw()


def zoom_in():
    global current_zoom, current_xstep, current_ystep

    current_zoom *= 1.5
    current_xstep /= current_zoom
    current_ystep /= current_zoom

    send_float(current_xstep, NamedRegisters.STEP_X.value)
    send_float(current_ystep, NamedRegisters.STEP_Y.value)
    calc_and_redraw()


def zoom_out():
    global current_zoom, current_xstep, current_ystep

    current_zoom *= 1 / 1.5
    current_xstep /= current_zoom
    current_ystep /= current_zoom

    send_float(current_xstep, NamedRegisters.STEP_X.value)
    send_float(current_ystep, NamedRegisters.STEP_Y.value)
    calc_and_redraw()


def reset_and_redraw():
    global current_left, current_up, current_zoom, current_xstep, current_ystep, stepping_factor
    current_left = DEFAULT_CURRENT_LEFT
    current_up = DEFAULT_CURRENT_UP
    current_zoom = DEFALT_CURRENT_ZOOM
    current_xstep = DEFAULT_XSTEP
    current_ystep = DEFAULT_YSTEP
    stepping_factor = DEFAULT_STEPPING

    send_defaults()
    calc_and_redraw()


kb.add_hotkey('r', lambda: reset_and_redraw())
kb.add_hotkey('a', lambda: pan_left())
kb.add_hotkey('d', lambda: pan_right())
kb.add_hotkey('w', lambda: pan_up())
kb.add_hotkey('s', lambda: pan_down())
kb.add_hotkey('f', lambda: zoom_in())
kb.add_hotkey('g', lambda: zoom_out())
kb.add_hotkey('up', lambda: increase_stepping_factor())
kb.add_hotkey('down', lambda: decrease_stepping_factor())

calc_and_redraw()

while True:
    # Press Q on keyboard to  exit
    if cv.waitKey(25) & 0xFF == ord('q'):
        break
    cv.imshow("Mandelbrot", data)

cv.destroyAllWindows()

RENDER_NAME = "half_fast"

# cv.imwrite(f'renders/{RENDER_NAME}.bmp', data)

# img.save(f'renders/{RENDER_NAME}.bmp')
