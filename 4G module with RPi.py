import serial
import time

# Initialize Serial port for communication 4G module 
esp_serial = serial.Serial('/dev/ttyAMA0', 115200, timeout=1)

def send_at_command(command):
    # Print the command
    print("Sending command:", command)
    # Send the command to 4G 
    esp_serial.write((command + '\r\n').encode())
    # Wait for response
    time.sleep(1)
    response = esp_serial.read_all().decode()
    # Print the response
    print(response)

# Main program
if __name__ == "__main__":
    # Wait for user input
    while True:
        input_data = input("Press 'S' to send AT commands: ")
        if input_data.upper() == 'S':
            # Send AT command
            send_at_command("AT")
            # Send AT+CSQ command
            send_at_command("AT+CSQ")
