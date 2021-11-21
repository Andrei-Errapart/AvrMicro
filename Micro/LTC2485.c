// vim: ts=4 shiftwidth=4
#include <avr/io.h>
#include <util/delay.h>

#include <Micro/LTC2485.h>
#include <Micro/uart.h>

volatile uint8_t*	scl_port_	= &PORTC;
volatile uint8_t*	scl_ddr_	= &DDRC;
uint8_t				scl_mask_	= _BV(0);
volatile uint8_t*	sda_port_	= &PORTC;
volatile uint8_t*	sda_ddr_	= &DDRC;
volatile uint8_t*	sda_pin_	= &PINC;
uint8_t				sda_mask_	= _BV(1);

/** The one and only LTC248X in this circuit */
#define	LTC2485_ADDR	0b01001000

// bitwise OR with address for read
#define I2C_READ 0x01
// bitwise OR with address for write
#define I2C_WRITE 0x00

#define I2C_SDA(b) do {									\
						if ((b)) {						\
							*sda_ddr_	&= ~sda_mask_;	\
							*sda_port_	&= ~sda_mask_;	\
						} else { 			\
							*sda_ddr_	|= sda_mask_;	\
							*sda_port_	&= ~sda_mask_;	\
						}					\
				   } while(0)
#define I2C_SCL(b) do {						\
						if ((b)) {			\
							*scl_ddr_	&= ~scl_mask_;	\
							*scl_port_	&= ~sda_mask_;	\
						} else { 			\
							*scl_ddr_	|= scl_mask_;	\
							*scl_port_	&= ~sda_mask_;	\
						}					\
				   } while (0)

#define I2C_SDAi()  		(((*sda_pin_) & sda_mask_)==0 ? 0 : 1)
#define I2C_DELAY_US	1
// FIXME: add delay when not working.
#define	i2c_delay()	do {			\
		/* _delay_us(I2C_DELAY_US);*/	\
	} while (0)

/*****************************************************************************/
static void
I2C_Start (void)
{
	I2C_SDA(1);
	I2C_SCL(1);
	i2c_delay();
	I2C_SDA(0);
	i2c_delay();
}



/*****************************************************************************/
static void
I2C_Stop(void)
{
	I2C_SDA(0);
	i2c_delay();
	I2C_SCL(1);
	i2c_delay();
	I2C_SDA(1);
	i2c_delay();
}

/*****************************************************************************/
typedef enum {
	ACK,
	NOACK
} ACK_t;

/*****************************************************************************/
static uint8_t
I2C_Read(
	const ACK_t	ack
)
{
	char i, temp = 0;
	I2C_SCL(0);
	i2c_delay();
	I2C_SDA(1);
	for (i=0 ;i<8;i++){
		//_delay_us(I2C_DELAY_US);
		I2C_SCL(0);
		i2c_delay();
		I2C_SCL(1);

		i2c_delay();
		temp <<= 1;
		temp += I2C_SDAi();
	}

	I2C_SCL(0);
	i2c_delay();
	if (ack == ACK) {
		I2C_SDA(0);
	} else {
		I2C_SDA(1);
	} 
	i2c_delay();
	I2C_SCL(1);
	i2c_delay();
	I2C_SCL(0);

	return temp;
}

/*****************************************************************************/
static uint8_t
I2C_Write(
	uint8_t Data)
{
	uint8_t i;

	for (i=0 ;i<8;i++){
		I2C_SCL(0);
		i2c_delay();
		if (Data & 0x80) {
			I2C_SDA(1);
		} else {
			I2C_SDA(0);
		}
		i2c_delay();
		I2C_SCL(1);
		i2c_delay();
		Data<<=1;
	}

	I2C_SCL(0);
	i2c_delay();
	I2C_SDA(1);
	i2c_delay();
	I2C_SCL(1);
	i2c_delay();
	i = I2C_SDAi();
	I2C_SCL(0);
	return i;
}

/*****************************************************************************/
void 
LTC2485_Init(
	volatile uint8_t*	scl_port,
	const uint8_t		scl_bit,
	volatile uint8_t*	sda_port,
	const uint8_t		sda_bit
)
{
	scl_port_	= scl_port;
	scl_ddr_	= scl_port - 1;
	scl_mask_	= _BV(scl_bit);
	sda_port_	= sda_port;
	sda_ddr_	= sda_port - 1;
	sda_pin_	= sda_port - 2;
	sda_mask_	= _BV(sda_bit);

	// I2C init...
	I2C_SDA(1);
	I2C_SCL(1);

	// Perform first read.
	LTC2485_Read();
} // function LTC2485_init


/*****************************************************************************/
void 
LTC2485_SetConfig(
	const uint8_t	config
)
{
	// Start communication with LTC2485:
	I2C_Start();
	// Write config, if possible.
	if (I2C_Write(LTC2485_ADDR | I2C_WRITE) == 0) {
		I2C_Write(config);
	}
	I2C_Stop();
}

/*****************************************************************************/
uint32_t
LTC2485_Read(void)
{
	// Result, four consecutive bytes.
	uint32_t r = 0;

	I2C_Start();
	if (I2C_Write(LTC2485_ADDR | I2C_READ) == 0) {
		r = I2C_Read(ACK);
		r =r <<8;
		r += I2C_Read(ACK);
		r =r <<8;
		r += I2C_Read(ACK);
		r =r <<8;
		r += I2C_Read(ACK);
	}
	I2C_Stop();

	return r;
} // function LTC2485_read

