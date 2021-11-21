// vim: ts=4 shiftwidth=4

#include <avr/io.h>
#include <avr/interrupt.h>

#include <Micro/DAC8560.h>

// PORTB pin 4
#define	CS_PIN	4

#define SET_CS()  do {PORTB &= ~(1 << CS_PIN); } while(0)
#define CLR_CS()  do {PORTB |= (1 << CS_PIN); } while(0)

#define	SPI_Write(data)	do { SPDR = data; while (!(SPSR & (1<<SPIF))) ; } while (0)

#define	STEP_CMD	3
#define	STEP_MSB	2
#define	STEP_LSB	1
#define	STEP_IDLE	0

static volatile uint8_t		step = STEP_IDLE;
static volatile uint16_t	dac_out = 0;
static DAC8560_IRQ_USAGE	irq_usage = DAC8560_WITHOUT_IRQ;

/*****************************************************************************/
/* SPI Serial Transfer Complete */
ISR(SPI_STC_vect)
{
	if (step>0) {
		switch (step) {
		case STEP_CMD:
			SPDR = (uint8_t)(dac_out >> 8);
			break;
		case STEP_MSB:
			SPDR = (uint8_t)(dac_out & 0xFF);
			break;
		case STEP_LSB:
			SET_CS(); 
			break;
		}
		--step;
	}
}

/*****************************************************************************/
void
DAC8560_Init(
	const DAC8560_IRQ_USAGE	dac8560_irq_usage
)
{
	irq_usage = dac8560_irq_usage;
	// 1. Port setup.
	DDRB |= (1 << CS_PIN) | (1<<PB5) | (1<<PB7);

	// 2. SPI setup.
	SPSR = (1 << SPI2X);
	// interrupts disabled, SPI enabled, MSB first, Master
	// SCK is low when idle, leading edge is setup, clock frequency is 1/4 of CPU freq.
	SPCR =
		  (irq_usage == DAC8560_WITH_IRQ
		   	? (1 << SPIE)
			: 0 )
		| (1<<SPE)
		| (1<<MSTR)
		| (1<<CPHA); // 0x54;
}

/*****************************************************************************/
void
DAC8560_Write (
	const uint16_t	Data)
{
	switch (irq_usage) {
	case DAC8560_WITH_IRQ:
		if (step == STEP_IDLE) {
			// 1. start writing.
			CLR_CS();
			dac_out = Data;
			step = STEP_CMD;
			SPDR = 0x00;
		}
		break;
	case DAC8560_WITHOUT_IRQ:
		CLR_CS();
		SPI_Write(0);
		SPI_Write(Data >> 8);
		SPI_Write(Data & 0x00FF);
		SET_CS(); 
		break;
	}
}

/*****************************************************************************/
bool
DAC8560_Busy(void)
{
	switch (irq_usage) {
	case DAC8560_WITH_IRQ:
		return step != STEP_IDLE;
	case DAC8560_WITHOUT_IRQ:
		return false;
	}
	return false;
}

