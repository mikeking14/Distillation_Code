import pyrealtime as prt

serial_port = '/dev/cu.usbmodem14201';
baud_rate = 115200;


serial_layer = prt.SerialReadLayer(serial_port = serial_port, baud_rate=baud_rate)
prt.TimePlotLayer(serial_layer, window_size=100, ylim=(0, 100))
prt.LayerManager.session().run()
