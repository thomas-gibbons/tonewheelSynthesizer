#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

int i2c_start_bus(unsigned char bus);
void i2c_close_bus(int fd);
int i2c_write(int file, unsigned char addr, unsigned char reg, unsigned char *value, unsigned char num);
int i2c_write_byte(int file, unsigned char addr, unsigned char value);
int i2c_read(int file, unsigned char addr, unsigned char reg, unsigned char *value, unsigned char num);
int i2c_read_byte(int file, unsigned char addr,unsigned char *value);
int i2c_write_bytes(int file, unsigned char addr, unsigned char *value, unsigned char num);
int i2c_read_bytes(int file, unsigned char addr, unsigned char *value, unsigned char num);