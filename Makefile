HOST_IP = $(shell ifconfig | grep inet | grep 192.168 | awk -F' ' '{print $$2}')
MAIN_INO = esp-ble-gateway.ino

erase_flash:
	tools/esptool.py -p /dev/tty.SLAB_USBtoUART erase_flash

lib_update:
	pio lib update	

monitor:
	platformio serialports monitor --baud 115200

flash: 
	platformio run -t upload 

clean:
	platformio run -t clean 

test:
	pio run

all:flash
