#############
## Script listens to serial port and writes contents into a file
##############
## requires pySerial to be installed
import serial
import csv
distillation_run_number = 35


serial_port = '/dev/cu.usbmodem14201';
baud_rate = 115200;
file_path = "/Users/michaelking/Documents/PlatformIO/Projects/Distillation_Code/DataProcessing/data/Distilation" + str(distillation_run_number) + '.txt'

output_file = open(file_path, "w+");
ser = serial.Serial(serial_port, baud_rate)

#Include the headers for each variable
field_names = ["x_value", "total_1", "total_2"]
with open(file_path, 'w') as output_file:
    wr = csv.writer(output_file)
    wr.writerow(field_names)

while ser.inWaiting:
    line = ser.readline();
    line = line.decode("utf-8") #ser.readline returns a binary, convert to string
    with open(file_path, 'a') as output_file:
        output_file.write(line)
        print(line)

ser.close()

#making a change!
