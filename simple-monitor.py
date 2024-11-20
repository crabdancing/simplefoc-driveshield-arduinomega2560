#!/usr/bin/env python3

import serial
import threading
import sys
import readline  # Import readline for line editing
from colorama import Fore, Style, init

# Initialize colorama
init(autoreset=True)

# Configuration
SERIAL_PORT = '/dev/ttyACM'
BAUD_RATE = 115200

# Initialize serial port
for i in range(10):
    try:
        ser = serial.Serial(SERIAL_PORT + str(i), BAUD_RATE, timeout=0.1)
        break
    except Exception as e:
        print(f"{Fore.RED}Failed to open serial port: {e}")
else:
    sys.exit(1)

def colorize(text):
    """Applies color to letters and numbers."""
    colored_text = ""
    for char in text:
        if char.isdigit() or char == '.':  # Numbers and decimal points
            colored_text += f"{Fore.CYAN}{char}{Style.RESET_ALL}"
        elif char.isalpha():  # Letters
            colored_text += f"{Fore.YELLOW}{char}{Style.RESET_ALL}"
        else:  # Other characters (e.g., spaces, punctuation)
            colored_text += char
    return colored_text

def handle_serial_output():
    """Continuously read from the serial port and print output to the terminal."""
    try:
        while True:
            data = ser.read_until(b'\n')  # Read a line of data from the serial device
            if data:
                decoded_line = data.decode().strip()
                print(f"\r{colorize(decoded_line)}")  # Print colorized firmware output
                print(f"{Fore.GREEN}> {readline.get_line_buffer()}{Style.RESET_ALL}", end="", flush=True)  # Redraw the green input prompt
    except Exception as e:
        print(f"{Fore.RED}Error reading from serial: {e}")

def user_input_handler():
    """Handles user input and sends it to the serial port."""
    try:
        while True:
            line = input(f"{Fore.GREEN}> {Style.RESET_ALL}")  # Prompt user input in green
            ser.write((line + '\n').encode())  # Send the input to the serial port
    except EOFError:
        print(f"\n{Fore.RED}EOF received. Exiting...")
        sys.exit(0)
    except KeyboardInterrupt:
        print(f"\n{Fore.RED}Keyboard interrupt received. Exiting...")
        sys.exit(0)

# Start the output thread
output_thread = threading.Thread(target=handle_serial_output, daemon=True)
output_thread.start()

# Handle user input
user_input_handler()

