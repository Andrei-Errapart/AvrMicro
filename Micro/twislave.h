// vim: ts=4 shiftwidth=4
#ifndef Micro_twislave_h_
#define Micro_twislave_h_

/** \file
 * TWI slave interface, interrupt-based. For a TWI master the device is presented
 * as an array of 8-bit registers, 256 registers in total. Register contents
 * are specified by user through two callback functions.
 * 
 * TWI master should write data <b>D</b> to the register <b>R</b> as follows:
 * <ol>
 *   <li><em>twi_write ADDR [R; D]</em>
 * </ol>
 * TWI master should read data from the register <b>R</b> as follows:
 * <ol>
 *   <li><em>twi_write ADDR [R]</em>
 *   <li><em>twi_read ADDR 1</em>
 * </ol>
 *
 * Usage:
 * <ol>
 * 	<li>Initialize TWI by calling <b>twislave_init</b>.
 * 	<li>Optional: activate internal pullups.
 * 	<li>Implement at least one of either <b>twislave_write_callback</b> or <b>twislave_read_callback</b> or both.
 * 	<li>Enable interrupts. The functions <b>twislave_write_callback</b> and <b>twislave_read_callback</b>
 * 	will be called as needed by the TWI interrupt service routine.
 * </ol>
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize TWI slave. Note: this function will not activate internal pullups.
 * \param[in]	address	TWI slave address, in the range [1..127].
 */
void
twislave_init(
	const uint8_t	address
);

/** Close TWI slave, i.e. disable TWI interrupt and the peripherial. */
void
twislave_close();

/** This function is called when TWI master writes to the register. It is run with interrupts disabled.
 * It is not terribly time-critical, however, long operations should be avoided.
 *
 * Implemented by user code, optionally.
 * \param[in]	register_no	Number of the register to be written.
 * \param[in]	data		Data written to the register.
 */
extern void
twislave_write_callback(
	const uint8_t	register_no,
	const uint8_t	data
) __attribute__ ((weak));

/** This function is called when TWI master reads from the register. It is run with interrupts disabled.
 * Long operations should be avoided.
 *
 * NB! Since the master might repeat the reads when answers do not arrive in time, do not make destructive changes
 * in this function. Incrementing an index register in this function can lead to loss of data.
 *
 * Implemented by user code, optionally. Register reads return zero when this function is not implemented.
 * \param[in]	register_no	Number of the register to be read.
 * \return		Value read from the register.
 */
extern uint8_t
twislave_read_callback(
	const uint8_t	register_no
) __attribute__ ((weak));

#ifdef __cplusplus
}
#endif

#endif /* Micro_twislave_h_ */

