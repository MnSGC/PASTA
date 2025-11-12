import sys
from time import sleep
import Adafruit_GPIO.SPI as SPI
import Adafruit_MCP3008

SPI_TYPE = 'SW' #setting the SPI to the software version 
dly = 0.5 #delay time

CLK = 40  #CLOCK: a pin that provides timing info 
MISO = 19 #MASTER INPUT SLAVE OUTPUT: a pin the Pi uses to send info to the ADC
MOSI = 21 #MASTER OUTPUT SLAVE INPUT: a pin the Pi uses to receive data from ADC
CS = 26   #CHIP SELECT: a pin the Pi uses to indicate which device it's talking to 

HW_SPI_PORT = 0 #Using SPI bus 0
HW_SPI_DEV = 0 #Using device 0 on SPI bus 0

if (SPI_TYPE == 'HW'): #checking to see if the SPI protocol is using the hardware version
    mcp = Adafruit_MCP3008.MCP3008(spi=SPI.SpiDev(HW_SPI_PORT, HW_SPI_DEV))
else:
    mcp = Adafruit_MCP3008.MCP3008(clk=CLK, cs = CS, miso = MISO, mosi = MOSI)

if (len(sys.argv)<=1):
    print("Usage: MCP3008.py <Analog Port>")
    sys.exit(1)
else:
    analogPort = int(sys.argv[1])
    print ('Reading MCP3008 values on pin: %d' % analogPort) 

while True:
    val = mcp.read_adc(analogPort) #returning the current ADC value from that channel (e.g analogPort)
    
    print(val) 
    
    sleep(dly)

