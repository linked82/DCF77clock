This project is an implementation of the interpretation of received DCF77 data in Madrid, Spain, Europe. 
This was made from the scratch using the Launchpad microcontroller. 

The hardware has the following characteristics:
  - Platform MSP430 Launchpad
  - Processor MSP430G2553 @ 8MHz
  - DCF77 clock receiver from Conrad
    http://www.conrad.com/ce/en/product/641138/DCF-receiver-board
    
The DCF77 module is hard wire connected to the Launchpad  MSP-EXP430G2 using wires, as shown:
  - VCC		3.3V (using an external regulator)
  - GND		Module Ground
  - P2.0	Inverting output. Connected to interrupt, with pull up enabled.
  - P2.1	Inverting output. Connected to CCR1 register, with pull up enabled.

The software is capable of
  - Receiving DCF77 date & time
  - Checking parity bits

The next implementation will be to display time on the screen instead of the debugger

Have fun and remember to use at your own risk, given that I don't guarantee that this crap won't burn your house.