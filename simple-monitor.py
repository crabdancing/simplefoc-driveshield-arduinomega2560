#!/usr/bin/env python3

import serial
import threading
import sys
from queue import Queue

# Configuration
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

# Initialize serial port
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)

def handle_serial_output():
    """Continuously read from the serial port and print output to the terminal."""
    while True:
        try:
            data = ser.read_until(b'\n')  # Read a line of data from the serial device
            if data:
                print("\r" + data.decode().strip())  # Print the received line
                print("> ", end="", flush=True)  # Redisplay the input prompt
        except Exception as e:
            print(f"Error reading from serial: {e}")
            break

def user_input_handler():
    """Handles user input and sends it to the serial port."""
    try:
        while True:
            line = input("> ")  # Prompt user for input
            ser.write((line + '\n').encode())  # Send the input to the serial port
    except KeyboardInterrupt:
        print("\nExiting...")
        sys.exit(0)

# Start the output thread
output_thread = threading.Thread(target=handle_serial_output, daemon=True)
output_thread.start()

# Handle user input
user_input_handler()
