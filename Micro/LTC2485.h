// vim: ts=4 shiftwidth=4
#ifndef LTC2485_h_
#define LTC2485_h_

/** \file
 * LTC2485 interface over software TWI. One reading takes 0.5 milliseconds.
 *
 * TODO: Separate software TWI.
 *
 * TODO: Permit setting speed of transmission.
 */
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Input: differential.
#define LTC2485_VIN 0b00000000
/// Input: PTAT circuit.
#define LTC2485_PTAT 0b00001000
/// Rejection frequency: 50 Hz.
#define LTC2485_R50 0b00000010
/// Rejection frequency: 55 Hz.
#define LTC2485_R55 0b00000000
/// Rejection frequency: 60 Hz.
#define LTC2485_R60 0b00000100
/// Speed mode: slow output rate with autozero.
#define LTC2485_SLOW 0b00000000
/// Speed mode: fast output rate with no autozero
#define LTC2485_FAST 0b00000001

/** Initialize LTC2485 port and start first conversion.
 * \param[in]	scl_port	SCL port (for example, &PORTC).
 * \param[in]	scl_bit		Bit index in the SCL port (for example, 4).
 * \param[in]	sda_port	SDA port (for example, &PORTC).
 * \param[in]	sda_bit		Bit index in the SDA port (for example, 5).
 *
 */
void 
LTC2485_Init(
	volatile uint8_t*	scl_port,
	const uint8_t		scl_bit,
	volatile uint8_t*	sda_port,
	const uint8_t		sda_bit
);

/** Set ADC configuration register.
 * \param[in]	config	Bitwise combination of LTC2485_XYZ flags.
 */
void 
LTC2485_SetConfig(
	const uint8_t	config
);

/** Read ADC conversion register and start new conversion.
 *
 * Time: 0.5 milliseconds.
 * \return		24-bit reading. 0 if no conversion result present or LTC2485 missing.
 */
uint32_t
LTC2485_Read(void);

#if defined(__cplusplus)
}
#endif

#endif /* LTC2485_h_ */

