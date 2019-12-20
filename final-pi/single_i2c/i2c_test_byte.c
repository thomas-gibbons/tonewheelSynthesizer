#include <stdio.h>
#include <math.h>
#include "i2c.h"

#define ADDR  0x25

int main(void) {
    int bus;
    unsigned char tx=0;
    unsigned char rx=0;

    bus = i2c_start_bus(1);

    while (1) {
        i2c_write_bytes(bus, ADDR, &tx,1);
		
        i2c_read_bytes(bus, ADDR,&rx,1);
		
        printf("TX: %03d  RX:%03d\n", (int)tx, (int)rx);

        usleep(200*1000);
		
		tx += 1;
	}
	
    i2c_close_bus(bus);
    return 0;
}
