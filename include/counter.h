#define I2C_COUNTER_ADDRESS 0x15


typedef enum
{
	ERROR_OK = 0,
	ERROR_NO_ANSWER = 2,
	ERROR_LINE_BUSY = 4,
}counter_error_t;

static int last_error = 0;
static uint16_t last_value = 0;


uint16_t counter_read()
{
  	uint16_t counter;
	last_error = twi_readFrom(I2C_COUNTER_ADDRESS, (uint8_t*)&counter, 2, true);
	return counter;
}

void counter_init()
{
	last_value = counter_read();
}

/**
 *  Returns last read counter difference with overflow handling
 */
uint16_t counter_read_diff()
{
	uint16_t diff = last_value;
	last_value = counter_read();
	return last_value - diff;
}

int counter_get_last_error()
{
	return last_error;
}



