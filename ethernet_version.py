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
host = "192.168.1.10"
port = 7
host2 = "192.168.1.3"
port2 = 7

# RENDERING SETTINGS
PIXELS_PER_LINE_LOW = 320
TOTAL_Y_LOW = 240
PIXELS_PER_LINE = 640
TOTAL_Y = 480

# FIXED POINT CONSTANTS - Using calculated values
DEFAULT_LEFT = -2
DEFAULT_UP = 1
DEFAULT_ZOOM = 1
DEFAULT_XSTEP = -3 / PIXELS_PER_LINE
DEFAULT_YSTEP = 2 / TOTAL_Y
DEFAULT_STEPPING = 0.1

# Pre-calculated FixedPoint constants
FP_NEG_TWO = FixedPoint(DEFAULT_LEFT, signed=True, m=12, n=52)
FP_ONE = FixedPoint(DEFAULT_UP, signed=True, m=12, n=52)
FP_ZERO = FixedPoint(0, signed=True, m=12, n=52)
FP_DEFAULT_XSTEP = FixedPoint(DEFAULT_XSTEP, signed=True, m=12, n=52)
FP_DEFAULT_YSTEP = FixedPoint(DEFAULT_YSTEP, signed=True, m=12, n=52)
FP_ONE_POINT_FIVE = FixedPoint(1.5, signed=True, m=12, n=52)
FP_STEPPING = FixedPoint(DEFAULT_STEPPING, signed=True, m=12, n=52)
FP_ONE_POINT_ONE = FixedPoint(1.1, signed=True, m=12, n=52)
FP_ZERO_POINT_NINE = FixedPoint(0.9, signed=True, m=12, n=52)

# create sockets
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)

s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s2.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
s2.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)

# connect
s.connect((host, port))
print("connected to 1")

sockets = [s]

# Mouse callback variables for rectangle selection
drawing = False
rect_start = (-1, -1)
rect_end = (-1, -1)
temp_img = None
selection_complete = False

# colour vars

colour_r, colour_g, colour_b = 0, 215, 255


class MandelbrotStreamingClient:
    def __init__(self, socket_obj):
        self.sock = socket_obj
        self.buffer = b""

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
            self.buffer = b""

    def receive_until_delimiter(self, delimiter=b"\n"):
        """Receive data until we find the delimiter"""
        while delimiter not in self.buffer:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("Socket closed before receiving delimiter")
            self.buffer += chunk

        # Split at first delimiter
        line, self.buffer = self.buffer.split(delimiter, 1)
        return line

    def receive_exact(self, length):
        """Receive exactly 'length' bytes"""
        while len(self.buffer) < length:
            chunk = self.sock.recv(length - len(self.buffer))
            if not chunk:
                raise ConnectionError("Socket closed before receiving all data")
            self.buffer += chunk

        # Extract the required amount
        data = self.buffer[:length]
        self.buffer = self.buffer[length:]
        return data

    def start_streaming(self, low_res: bool = False):
        """Start the streaming process and return stream info"""
        self.flush_socket_buffer()
        if low_res:
            self.sock.send("LOWRE".encode())
        else:
            self.sock.send("STREM".encode())

        # Wait for stream start confirmation
        response = self.receive_until_delimiter()
        response_str = response.decode()

        if not response_str.startswith("STREAM_START"):
            raise ValueError(f"Unexpected response: {response_str}")

        # Parse total lines from response: "STREAM_START:LINES:1080"
        match = re.search(r"LINES:(\d+)", response_str)
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
        parts = line_str.split(":", 2)
        if len(parts) != 3:
            raise ValueError(f"Invalid line format: {line_str[:50]}...")

        line_number = int(parts[1])
        hex_data = parts[2]

        # Convert hex string to byte array
        if len(hex_data) % 2 != 0:
            raise ValueError(f"Hex data length not even: {len(hex_data)}")

        # Convert hex pairs to integers
        int_array = [int(hex_data[i : i + 2], 16) for i in range(0, len(hex_data), 2)]

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
                full_image = np.zeros((total_lines, line_length), dtype=np.uint8)
                print(f"Image dimensions: {total_lines} x {line_length}")

            # Store the line data
            if line_number < total_lines:
                full_image[line_number] = line_data
                received_lines += 1

                if received_lines % 100 == 0:
                    elapsed = time() - start_time
                    progress = (received_lines / total_lines) * 100
                    print(
                        f"Progress: {progress:.1f}% ({received_lines}/{total_lines}) - {elapsed:.2f}s"
                    )

        elapsed = time() - start_time
        print(f"Stream complete! Received {received_lines} lines in {elapsed:.2f}s")

        return full_image


class NamedRegisters(Enum):
    CURRENT_X = "CURRX"
    CURRENT_Y = "CURRY"
    STEP_X = "XSTEP"
    STEP_Y = "YSTEP"
    TOP_LEFT_X = "X_TOP"
    TOP_LEFT_Y = "Y_TOP"
    Z_RE = "ZREAL"
    Z_IM = "ZIMAG"


def flush_socket_buffer(incoming_socket):
    """Clear any remaining data in the socket buffer"""
    incoming_socket.settimeout(0.1)
    try:
        while True:
            data = incoming_socket.recv(1024)
            if not data:
                break
    except socket.timeout:
        pass
    finally:
        incoming_socket.settimeout(None)


def check_reg(reg: str):
    valid_regs: list[str] = [e.value for e in NamedRegisters]
    if reg not in valid_regs:
        raise ValueError(
            f"Invalid register to send to, valid registers are {valid_regs}"
        )


def custom_decode(bytes_val: bytes):
    return str(bytes_val).replace("\\x", "").replace("'", "").strip("b").upper()


def send_float(value: Union[float, str, FixedPoint], reg: str, socket=s):
    check_reg(reg)

    if isinstance(value, FixedPoint):
        data_bytes = value.bits  # Extract the bits from FixedPoint
    else:
        fp = FixedPoint(value, signed=True, m=12, n=52, str_base=16)
        data_bytes = fp.bits  # Extract the bits from the newly created FixedPoint

    bits_value = data_bytes & 0xFFFFFFFFFFFFFFFF

    data_bytes = bytes(struct.pack(">Q", bits_value))
    message = (
        reg.encode() + " ".encode() + data_bytes + chr(4).encode() + chr(4).encode()
    )
    socket.send(message)


def fetch_float(reg: str, ret_hex: bool = False, socket=s):
    check_reg(reg)
    flush_socket_buffer(socket)
    print(f"Fetching reg {reg}")

    message = reg.encode()
    socket.send(message)

    socket.settimeout(2.0)
    msg = socket.recv(32)

    if ret_hex:
        return custom_decode(msg)

    return float(FixedPoint("0x" + (custom_decode(msg)), signed=True, m=12, n=52))


def zoom_to_rectangle():
    """Zoom into the selected rectangle area"""
    global rect_start, rect_end, current_left, current_up, current_xstep, current_ystep, selection_complete

    if not selection_complete:
        print("No rectangle selected!")
        return

    # Ensure rectangle coordinates are valid
    x1, y1 = rect_start
    x2, y2 = rect_end

    # Make sure we have proper min/max
    if x1 > x2:
        x1, x2 = x2, x1
    if y1 > y2:
        y1, y2 = y2, y1

    # Prevent zero or negative width/height rectangles
    if x2 - x1 < 5 or y2 - y1 < 5:
        print("Rectangle too small for meaningful zoom!")
        return

    print(f"Rectangle pixels: ({x1}, {y1}) to ({x2}, {y2})")
    print(f"Current view: left={float(current_left)}, up={float(current_up)}")
    print(f"Current steps: x={float(current_xstep)}, y={float(current_ystep)}")

    # Calculate the real-world coordinates of the rectangle
    # The current view spans from current_left to (current_left + PIXELS_PER_LINE * current_xstep)
    # and from current_up to (current_up - TOTAL_Y * current_ystep)

    rect_left = float(current_left) - (x1 * float(current_xstep))
    rect_right = float(current_left) - (x2 * float(current_xstep))
    rect_top = float(current_up) - (y1 * float(current_ystep))  # y1 is top pixel
    rect_bottom = float(current_up) - (y2 * float(current_ystep))  # y2 is bottom pixel

    # Calculate new parameters
    rect_width = abs(rect_right - rect_left)
    rect_height = abs(rect_top - rect_bottom)
    # Should be positive (top > bottom in mandelbrot coords)

    print(f"Rectangle mandelbrot coords: left={rect_left}, right={rect_right}")
    print(f"Rectangle mandelbrot coords: top={rect_top}, bottom={rect_bottom}")
    print(f"Rectangle size: width={rect_width}, height={rect_height}")

    # Ensure we have positive dimensions
    if rect_width <= 0 or rect_height <= 0:
        print("Invalid rectangle dimensions in mandelbrot space!")
        return

    # Update the view parameters
    current_left = FixedPoint(
        rect_left if rect_left < rect_right else rect_right, signed=True, m=12, n=52
    )
    current_up = FixedPoint(rect_top, signed=True, m=12, n=52)  # Use rect_top, not max
    current_xstep = FixedPoint(-rect_width / PIXELS_PER_LINE, signed=True, m=12, n=52)
    current_ystep = FixedPoint(rect_height / TOTAL_Y, signed=True, m=12, n=52)

    print(f"New view: left={float(current_left)}, up={float(current_up)}")
    print(f"New steps: x={float(current_xstep)}, y={float(current_ystep)}")

    # Send new parameters to the server
    send_float(current_left, NamedRegisters.TOP_LEFT_X.value)
    print(f"fetched top left x value = {fetch_float(NamedRegisters.TOP_LEFT_X.value)}")
    send_float(current_up, NamedRegisters.TOP_LEFT_Y.value)
    print(f"fetched top left y value = {fetch_float(NamedRegisters.TOP_LEFT_Y.value)}")
    send_float(current_xstep, NamedRegisters.STEP_X.value)
    print(f"current_xstep = {fetch_float(NamedRegisters.STEP_X.value)}")
    send_float(current_ystep, NamedRegisters.STEP_Y.value)
    print(f"current_ystep = {fetch_float(NamedRegisters.STEP_Y.value)}")

    # Clear selection
    selection_complete = False

    # Trigger recalculation and redraw
    calc_and_redraw()


def mouse_callback(event, x, y, flags, param):
    """Mouse callback for rectangle selection - improved version with aspect ratio constraint"""
    global drawing, rect_start, rect_end, temp_img, data, selection_complete, rect_start, rect_end, current_left, current_up, current_xstep, current_ystep

    def constrain_aspect_ratio(start_point, end_point, target_ratio=1.33):
        """Constrain rectangle to maintain aspect ratio of target_ratio:1 (width:height)"""
        start_x, start_y = start_point
        end_x, end_y = end_point

        # Calculate current dimensions
        width = abs(end_x - start_x)
        height = abs(end_y - start_y)

        # If width is 0, return original points to avoid division by zero
        if width == 0:
            return end_point

        # Calculate what height should be based on width and target ratio
        target_height = width / target_ratio

        # Calculate what width should be based on height and target ratio
        target_width = height * target_ratio

        # Choose the constraint that results in a smaller rectangle (fits within current bounds)
        if target_height <= height:
            # Use width-based constraint
            new_height = int(target_height)
            # Maintain the direction of the drag
            if end_y >= start_y:
                new_end_y = start_y + new_height
            else:
                new_end_y = start_y - new_height
            return (end_x, new_end_y)
        else:
            # Use height-based constraint
            new_width = int(target_width)
            # Maintain the direction of the drag
            if end_x >= start_x:
                new_end_x = start_x + new_width
            else:
                new_end_x = start_x - new_width
            return (new_end_x, end_y)

    if event == cv.EVENT_LBUTTONDOWN:
        drawing = True
        rect_start = (x, y)
        rect_end = (x, y)
        temp_img = data.copy()
        selection_complete = False
        print(f"Started rectangle at ({x}, {y})")
        if flags & cv.EVENT_FLAG_SHIFTKEY:
            print("Shift held - constraining to 1.33:1 aspect ratio")

    elif event == cv.EVENT_MOUSEMOVE:
        if drawing and temp_img is not None:
            temp_img = data.copy()

            # Check if Shift is held for aspect ratio constraint
            if flags & cv.EVENT_FLAG_SHIFTKEY:
                rect_end = constrain_aspect_ratio(rect_start, (x, y))
            else:
                rect_end = (x, y)

            # Draw rectangle on temporary image
            cv.rectangle(temp_img, rect_start, rect_end, (255, 255, 255), 2)
            # Add corner markers for better visibility
            cv.circle(temp_img, rect_start, 3, (255, 255, 255), -1)
            cv.circle(temp_img, rect_end, 3, (255, 255, 255), -1)

    elif event == cv.EVENT_LBUTTONUP:
        if drawing:
            drawing = False

            # Apply final aspect ratio constraint if Shift was held
            if flags & cv.EVENT_FLAG_SHIFTKEY:
                rect_end = constrain_aspect_ratio(rect_start, (x, y))
            else:
                rect_end = (x, y)

            # Only mark as complete if rectangle has meaningful size
            width = abs(rect_end[0] - rect_start[0])
            height = abs(rect_end[1] - rect_start[1])
            if width >= 5 and height >= 5:
                temp_img = data.copy()
                cv.rectangle(temp_img, rect_start, rect_end, (255, 255, 255), 2)
                cv.circle(temp_img, rect_start, 3, (255, 255, 255), -1)
                cv.circle(temp_img, rect_end, 3, (255, 255, 255), -1)
                selection_complete = True
                aspect_ratio = width / height if height > 0 else 0
                print(
                    f"Rectangle selected: ({rect_start[0]}, {rect_start[1]}) to ({rect_end[0]}, {rect_end[1]})"
                )
                print(
                    f"Dimensions: {width}x{height}, Aspect ratio: {aspect_ratio:.2f}:1"
                )
                print("Press 'z' to zoom into selected area or 'c' to cancel selection")
            else:
                print("Rectangle too small - selection cancelled")
                temp_img = None
                selection_complete = False


# Additional helper function to debug current view parameters
def print_current_view():
    """Print current view parameters for debugging"""
    print("=== Current View Parameters ===")
    print(f"Left: {float(current_left)}")
    print(f"Up: {float(current_up)}")
    print(f"X Step: {float(current_xstep)}")
    print(f"Y Step: {float(current_ystep)}")
    print(f"Right edge: {float(current_left) - PIXELS_PER_LINE * float(current_xstep)}")
    print(f"Bottom edge: {float(current_up) - TOTAL_Y * float(current_ystep)}")
    print(f"View width: {PIXELS_PER_LINE * float(current_xstep)}")
    print(f"View height: {TOTAL_Y * float(current_ystep)}")
    print("==============================")


# Add hotkey for debugging (optional)
kb.add_hotkey("p", lambda: print_current_view())


def clear_selection():
    """Clear the current rectangle selection"""
    global selection_complete, temp_img
    selection_complete = False
    temp_img = None
    print("Selection cleared")


# Legacy functions for compatibility
def receive_exact(sock, length):
    client = MandelbrotStreamingClient(sock)
    return client.receive_exact(length)


def get_line_streaming(sock, low_res: bool = False):
    client = MandelbrotStreamingClient(sock)
    if low_res:
        return client.get_full_mandelbrot_stream(True)
    else:
        return client.get_full_mandelbrot_stream()


def reset():
    for socket in sockets:
        flush_socket_buffer(socket)
        socket.send("RESET".encode())


def calculate():

    send_float(0.0, NamedRegisters.CURRENT_Y.value)

    print(
        f"Current coordinates = {fetch_float(NamedRegisters.CURRENT_X.value)}, {fetch_float(NamedRegisters.CURRENT_Y.value)} "
    )

    for socket in sockets:
        flush_socket_buffer(socket)
        socket.send("CALCE".encode())

    time_start = time_ns()
    s.settimeout(100.0)
    msg = s.recv(32)
    time_end = time_ns()

    print(msg.decode())
    print(f"Took {(time_end - time_start) / 10 ** 9} seconds to calculate!")


def send_defaults():
    send_float(FP_NEG_TWO, NamedRegisters.TOP_LEFT_X.value)
    print(fetch_float(NamedRegisters.TOP_LEFT_X.value))
    send_float(FP_ONE, NamedRegisters.TOP_LEFT_Y.value)
    print(fetch_float(NamedRegisters.TOP_LEFT_Y.value))
    send_float(FP_DEFAULT_XSTEP, NamedRegisters.STEP_X.value)
    print(fetch_float(NamedRegisters.STEP_X.value))
    send_float(FP_DEFAULT_YSTEP, NamedRegisters.STEP_Y.value)
    print(fetch_float(NamedRegisters.STEP_Y.value))
    send_float(FP_ZERO, NamedRegisters.CURRENT_X.value)
    send_float(FP_ZERO, NamedRegisters.CURRENT_Y.value)


send_defaults()

# Initialize with proper FixedPoint constants
data = np.zeros([TOTAL_Y, PIXELS_PER_LINE], np.uint8)
current_left = FP_NEG_TWO
current_up = FP_ONE
current_xstep = FP_DEFAULT_XSTEP
current_ystep = FP_DEFAULT_YSTEP
stepping_factor = FP_STEPPING


def calc_and_redraw():
    global data, temp_img
    send_float(0.0, NamedRegisters.CURRENT_X.value)
    send_float(0.0, NamedRegisters.CURRENT_Y.value)
    calculate()
    new_data = get_line_streaming(s)
    data = np.zeros([new_data.shape[0], new_data.shape[1], 3], np.uint8)
    temp_img = None

    # Define special color for maximum values (255)
    max_value_color_r = 0  # Red component for value 255
    max_value_color_g = 0  # Green component for value 255
    max_value_color_b = 0  # Blue component for value 255 (white)

    # Create masks for different value ranges
    max_mask = new_data == 255
    normal_mask = new_data < 255

    # For values 0-254: interpolate between 0 and the target color
    normalized_data = (
        new_data.astype(np.float32) / 254.0
    )  # Normalize to [0,1] for range 0-254
    normalized_data = np.clip(normalized_data, 0, 1)  # Ensure we don't exceed 1

    # Apply normal color mapping for values < 255
    data[normal_mask, 0] = (normalized_data[normal_mask] * colour_b).astype(
        np.uint8
    )  # Blue
    data[normal_mask, 1] = (normalized_data[normal_mask] * colour_g).astype(
        np.uint8
    )  # Green
    data[normal_mask, 2] = (normalized_data[normal_mask] * colour_r).astype(
        np.uint8
    )  # Red

    # Apply special color for maximum values (255)
    data[max_mask, 0] = max_value_color_b  # Blue channel
    data[max_mask, 1] = max_value_color_g  # Green channel
    data[max_mask, 2] = max_value_color_r  # Red channel


def pan_left():
    global current_left, stepping_factor
    current_left = FixedPoint(
        float(current_left) - float(stepping_factor), signed=True, m=12, n=52
    )
    send_float(current_left, NamedRegisters.TOP_LEFT_X.value)
    print(f"fetched left value = {fetch_float(NamedRegisters.TOP_LEFT_X.value)}")
    calc_and_redraw()


def pan_right():
    global current_left, stepping_factor
    current_left = FixedPoint(
        float(current_left) + float(stepping_factor), signed=True, m=12, n=52
    )
    send_float(current_left, NamedRegisters.TOP_LEFT_X.value)
    print(f"fetched left value = {fetch_float(NamedRegisters.TOP_LEFT_X.value)}")
    calc_and_redraw()


def pan_up():
    global current_up, stepping_factor
    current_up = FixedPoint(
        float(current_up) + float(stepping_factor), signed=True, m=12, n=52
    )
    send_float(current_up, NamedRegisters.TOP_LEFT_Y.value)
    print(f"fetched up value = {fetch_float(NamedRegisters.TOP_LEFT_Y.value)}")
    calc_and_redraw()


def pan_down():
    global current_up, stepping_factor
    current_up = FixedPoint(
        float(current_up) - float(stepping_factor), signed=True, m=12, n=52
    )
    send_float(current_up, NamedRegisters.TOP_LEFT_Y.value)
    print(f"fetched up value = {fetch_float(NamedRegisters.TOP_LEFT_Y.value)}")
    calc_and_redraw()


def zoom_in():
    global current_xstep, current_ystep
    current_xstep = FixedPoint(float(current_xstep) / 1.5, signed=True, m=12, n=52)
    current_ystep = FixedPoint(float(current_ystep) / 1.5, signed=True, m=12, n=52)
    send_float(current_xstep, NamedRegisters.STEP_X.value)
    send_float(current_ystep, NamedRegisters.STEP_Y.value)
    calc_and_redraw()


def zoom_out():
    global current_xstep, current_ystep
    current_xstep = FixedPoint(float(current_xstep) * 1.5, signed=True, m=12, n=52)
    current_ystep = FixedPoint(float(current_ystep) * 1.5, signed=True, m=12, n=52)
    send_float(current_xstep, NamedRegisters.STEP_X.value)
    send_float(current_ystep, NamedRegisters.STEP_Y.value)
    calc_and_redraw()


def reset_and_redraw():
    global current_left, current_up, current_xstep, current_ystep, stepping_factor
    current_left = FP_NEG_TWO
    current_up = FP_ONE
    current_xstep = FP_DEFAULT_XSTEP
    current_ystep = FP_DEFAULT_YSTEP
    stepping_factor = FP_STEPPING
    send_defaults()
    calc_and_redraw()


current_colour = 0


def increase_colour():
    global colour_r, colour_g, colour_b
    if current_colour == 0:
        if colour_r < 255:
            colour_r += 1
    elif current_colour == 1:
        if colour_g < 255:
            colour_g += 1
    elif current_colour == 2:
        if colour_b < 255:
            colour_b += 1


def decrease_colour():
    global colour_r, colour_g, colour_b
    if current_colour == 0:
        if colour_r > 0:
            colour_r -= 1
    elif current_colour == 1:
        if colour_g > 0:
            colour_g -= 1
    elif current_colour == 2:
        if colour_b > 0:
            colour_b -= 1


def increment_current_colour():
    global current_colour
    current_colour += 1
    if current_colour > 2:
        current_colour = 0


def decrement_current_colour():
    global current_colour
    current_colour -= 1
    if current_colour < 0:
        current_colour = 2


# Keyboard shortcuts
kb.add_hotkey("r", lambda: reset_and_redraw())
# kb.add_hotkey("a", lambda: pan_left())
# kb.add_hotkey("d", lambda: pan_right())
# kb.add_hotkey("w", lambda: pan_up())
# kb.add_hotkey("s", lambda: pan_down())
# kb.add_hotkey("f", lambda: zoom_in())
# kb.add_hotkey("g", lambda: zoom_out())
kb.add_hotkey("up", lambda: increase_colour())
kb.add_hotkey("down", lambda: decrease_colour())
kb.add_hotkey("left", lambda: decrement_current_colour())
kb.add_hotkey("right", lambda: increment_current_colour())
kb.add_hotkey("z", lambda: zoom_to_rectangle())
kb.add_hotkey("c", lambda: clear_selection())

# Set up the window and mouse callback
cv.namedWindow("Mandelbrot")
cv.setMouseCallback("Mandelbrot", mouse_callback)

calc_and_redraw()

print("\nControls:")
print("Mouse: Click and drag to select rectangle area")
print("Z: Zoom into selected rectangle")
print("C: Clear selection")
print("WASD: Pan around")
print("F/G: Zoom in/out")
print("R: Reset to default view")
print("Up/Down arrows: Adjust pan speed")
print("Q: Quit")

while True:
    # Show either the temp image (with rectangle) or the main data
    display_img = temp_img if temp_img is not None else data
    cv.imshow("Mandelbrot", display_img)

    if cv.waitKey(25) & 0xFF == ord("q"):
        break

cv.destroyAllWindows()
