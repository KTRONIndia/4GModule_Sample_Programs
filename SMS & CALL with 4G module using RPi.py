import serial
import time
import logging

# %$Configure logging
logging.basicConfig(filename='communication.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

# Serial port setup
serial_port_Module = serial.Serial('/dev/ttyAMA0', 19200, timeout=1)  # Serial port for 4G Module

# Function to set up baud rate
def setup_baud():
    global baud
    if not baud:
        serial_port_Module.write(b'AT+IPR=19200\r\n')  # Set baud rate
        time.sleep(1)  # Wait for response
        serial_port_Module.flushInput()
        baud = True

# Function to send AT command and log response
def send_at_command(command):
    logging.info("Sending command: %s", command)
    print("Sending command:", command)
    serial_port_Module.write(command.encode() + b'\r\n')  # Send the command
    time.sleep(1)  # Wait for response
    response = serial_port_Module.read_all().decode().strip()  # Read the response
    logging.info("Response from 4G module for command '%s': %s", command, response)
    print("Response from 4G module for command : ", command, response)

# Function to make a call
def make_call(phone_number):
    logging.info("Making a call to %s...", phone_number)
    print("Making call to", phone_number)
    send_at_command('ATD' + phone_number + ';')

# Function to attend an incoming call
def attend_call():
    logging.info("Attending the call...")
    send_at_command('ATA')

# Function to send an SMS
def send_sms(phone_number, message):
    logging.info("Sending an SMS to %s...", phone_number)
    send_at_command('AT+CMGF=1')  # Set SMS text mode
    time.sleep(1)  # Wait for response
    send_at_command('AT+CMGS="' + phone_number + '"')  # Set SMS recipient
    time.sleep(1)  # Wait for response
    send_at_command(message + chr(26))  # Send the message

# Function to check for incoming messages and calls
def check_incoming():
    while True:
        response = serial_port_Module.readline().decode().strip()
        if response.startswith("+CMTI"):
            logging.info("New SMS received!")
            print("New SMS received!")
            # Extract index of the new SMS
            parts = response.split(',')
            newSMSIndex = parts[1].split('"')[0]
            print("Index of new SMS:", newSMSIndex)
            # Read the content of the new SMS using the obtained index
            send_at_command('AT+CMGF=1')  # Set SMS text mode
            time.sleep(1)  # Wait for response
            send_at_command('AT+CMGR=' + newSMSIndex)  # Read the SMS
        elif "RING" in response:
            logging.info("Incoming call detected!")
            print("Incoming call detected!")
            print("press 'a' to receive")
            # You can add code here to handle incoming call
        time.sleep(0.1)  # Wait before checking again

# Main program
if __name__ == "__main__":
    # Initialize flags and variables
    baud = False
    incomingCall = False
    outgoingCallDetected = False
    newSMSIndex = ""
    callingNumber = "+91**********"
    messageToSend = "Hello from Raspberry Pi!"

    # Start checking for incoming messages and calls in a separate thread
    import threading
    threading.Thread(target=check_incoming, daemon=True).start()

    # Wait for user input
    while True:
        command = input("Enter command (c for call, s for SMS, a for answer call): ")
        
        if command == 'c':
            setup_baud()
            make_call(callingNumber)
        elif command == 's':
            setup_baud()
            send_sms(callingNumber, messageToSend)
        elif command == 'a':
            setup_baud()
            attend_call()
        else:
            print("Invalid command")

