// vim: ts=4 shiftwidth=4
#include <Micro/twislave.h>	// ourselves
#include <util/twi.h>		// TWI bit mask definitions
#include <avr/interrupt.h>	// ISR

/****************************************************************************
  TWI State codes
****************************************************************************/
// General TWI Master staus codes                      
#define TWI_START                  0x08  // START has been transmitted  
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes                      
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received 
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received 

// TWI Master Receiver staus codes  
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = 0
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition

/*****************************************************************************/
void
twislave_init(
	const uint8_t	address
)
{
	TWBR = 0x00;
	TWAR = address << 1;
	TWAMR = 0x00;
	TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
}

/*****************************************************************************/
void
twislave_close()
{
	TWCR = 0x00;
}

/// Acknowledge received packet.
#define	TWCR_ACK	(_BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT))

/// Number of bytes received so far. This resets to zero at the start of the packet.
static uint8_t	rx_cnt = 0; 
/// Bytes received.
static uint8_t	rx_buf[4];
/// Register contents obtained by a call to 'twislave_read_callback', if any.
static uint8_t	tx_buf = 0;

/*****************************************************************************/
ISR(TWI_vect)
{
	switch (TWSR) {
	case TWI_STX_ADR_ACK:
		// Own SLA+R has been received; ACK has been returned
		rx_cnt = 0;
		TWDR = tx_buf;
		TWCR = TWCR_ACK;
		break;
	case TWI_STX_DATA_ACK:
		// Data byte in TWDR has been transmitted; ACK has been received
		TWDR = tx_buf;
		TWCR = TWCR_ACK;
		break;
	case TWI_STX_DATA_NACK:
		// Data byte in TWDR has been transmitted; NACK has been received. 
		// I.e. this could be the end of the transmission.
		TWCR =	TWCR_ACK;
		break;     
	case TWI_SRX_GEN_ACK:
		// General call address has been received; ACK has been returned
	case TWI_SRX_ADR_ACK:
		// Own SLA+W has been received ACK has been returned
		rx_cnt   = 0;
		TWCR = TWCR_ACK;
		break;
	case TWI_SRX_ADR_DATA_ACK:
		// Previously addressed with own SLA+W; data has been received; ACK has been returned
	case TWI_SRX_GEN_DATA_ACK:
		// Previously addressed with general call; data has been received; ACK has been returned
		if (rx_cnt < sizeof(rx_buf)) {
			rx_buf[rx_cnt] = TWDR;
			++rx_cnt;
		}
		TWCR = TWCR_ACK;
		break;
	case TWI_SRX_STOP_RESTART:
		// A STOP condition or repeated START condition has been received while still addressed as Slave    
		TWCR = TWCR_ACK; // this permits TWI hardware to continue working.
		switch (rx_cnt) {
		case 1:
			tx_buf = twislave_read_callback
				? twislave_read_callback(rx_buf[0])
				: 0;
			break;
		case 2:
			if (twislave_write_callback) {
				twislave_write_callback(rx_buf[0], rx_buf[1]);
			}
			break;
		}
		break;           
	case TWI_SRX_ADR_DATA_NACK:
		// Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
	case TWI_SRX_GEN_DATA_NACK:
		// Previously addressed with general call; data has been received; NOT ACK has been returned
	case TWI_STX_DATA_ACK_LAST_BYTE:
		// Last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received
	case TWI_BUS_ERROR:
		// Bus error due to an illegal START or STOP condition
	default:     
		TWCR = TWCR_ACK | _BV(TWSTO);
	}
}

