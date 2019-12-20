#include "i2c.h"

int i2c_start_bus(unsigned char bus) {
	char filename[11];
	int fd;
	
	sprintf(filename, "/dev/i2c-%d", bus);
	
	fd = open(filename, O_RDWR);
	
	if (fd < 0)
		fprintf(stderr, "Failed to communicate with I2C bus #%d.\n", bus);
		
	return fd;
}

void i2c_close_bus(int fd) {
	close(fd);
}

int i2c_write_byte(int file, unsigned char addr, unsigned char value) {
	int i;
	unsigned char outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr  = addr;
	messages[0].flags = 0;
	messages[0].len   = 1;
	messages[0].buf   = &outbuf;

	outbuf = value;

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs  = messages;
	packets.nmsgs = 1;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}

int i2c_write_bytes(int file, unsigned char addr, unsigned char *value, unsigned char num) {
	int i;
	unsigned char outbuf[256];
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr  = addr;
	messages[0].flags = 0;
	messages[0].len   = num;
	messages[0].buf   = outbuf;

	if (num > 255) {
		perror("Maximum Send num is 255");
		return 1;
	}
	for (i = 0; i <= num; i++)
		outbuf[i] = value[i];

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs  = messages;
	packets.nmsgs = 1;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}
	
int i2c_write(int file, unsigned char addr, unsigned char reg, unsigned char *value, unsigned char num) {
	int i;
	unsigned char outbuf[256];
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr  = addr;
	messages[0].flags = 0;
	messages[0].len   = num + 1;
	messages[0].buf   = outbuf;

	/* The first byte indicates which register we'll write */
	outbuf[0] = reg;

	/* 
	 * The remaining bytes indicate the values that will be written.
	 * The maximum number of data bytes allowed is 255.
	 */
	for (i = 1; i <= num; i++)
		outbuf[i] = value[i - 1];

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs  = messages;
	packets.nmsgs = 1;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}

int i2c_read(int file, unsigned char addr, unsigned char reg, unsigned char *value, unsigned char num) {
	unsigned char outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	/*
	 * In order to read a register, we first do a "dummy write" by writing
	 * 0 bytes to the register we want to read from.  This is similar to
	 * the packet in set_i2c_register, except it's 1 byte rather than 2.
	 */
	outbuf = reg;
	messages[0].addr  = addr;
	messages[0].flags = 0;
	messages[0].len   = sizeof(outbuf);
	messages[0].buf   = &outbuf;

	/* The data will get returned in this structure */
	messages[1].addr  = addr;
	messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
	messages[1].len   = num;
	messages[1].buf   = value;

	/* Send the request to the kernel and get the result back */
	packets.msgs	  = messages;
	packets.nmsgs	  = 2;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}


int i2c_read_byte(int file, unsigned char addr,unsigned char *value) {
	unsigned char outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	/* The data will get returned in this structure */
	messages[0].addr  = addr;
	messages[0].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
	messages[0].len   = 1;
	messages[0].buf   = value;

	/* Send the request to the kernel and get the result back */
	packets.msgs	  = messages;
	packets.nmsgs	  = 1;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}


int i2c_read_bytes(int file, unsigned char addr, unsigned char *value, unsigned char num) {
	unsigned char outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];


	/* The data will get returned in this structure */
	messages[0].addr  = addr;
	messages[0].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
	messages[0].len   = num;
	messages[0].buf   = value;

	/* Send the request to the kernel and get the result back */
	packets.msgs	  = messages;
	packets.nmsgs	  = 1;
	if(ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}
