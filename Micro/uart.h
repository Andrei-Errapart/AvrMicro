// vim: ts=2 shiftwidth=2
#ifndef uart_h_
#define uart_h_

/** \file Encapsulation of UART0 functions.
 */

#include <avr/io.h>				/* IO ports */
#include <stdint.h>				/* uint8_t */
#include <avr/pgmspace.h>	/* Program memory space. */

#if defined(__cplusplus)
extern "C" {
#endif

/** Implemented by user code, optionally. It is not required when you only write to UART. */
extern void uart_read_callback(const uint8_t	c) __attribute__ ((weak));

/** Calculate baud rate divisor.
 * \param[in]	f_cpu	CPU clock, Hz. Use F_CPU, if defined.
 * \param[in]	baud	Baud rate.
 */
#define	UART_BAUD_RATE_DIVISOR(f_cpu, baud)	((f_cpu)/((baud)*16l)-1)

/** Setup UART device RX/TX. For the RS485 ports, define macros UART_RS485_PORT and UART_RS485_PIN
 * on the compiler's command line, for example: -DUART_RS485_PORT=PORTD -DUART_RS485_PIN=0
 * \param[in]	baud_rate_divisor	Baud rate divisor, use UART_BAUD_RATE_DIVISOR to calculate one.
 */
extern void uart_setup(
	const uint16_t	baud_rate_divisor
);

/** Shut down UART hardware and disable UART interrupts. */
void uart_close();

/**
 * Print character to the uart.
 */
void uart_putchar(	const uint8_t	c);

/**
 * Print strings to debugging output.
 */
void uart_send_P( PGM_P					s);

/**
 * Print strings to debugging output.
 */
void uart_send(		const char*		s);

/**
 * Print newline (CRLF).
 */
void uart_send_crlf();

/** Write x in hexadecimal ASCII, two bytes, to the UART.
 */
void uart_send_hex08(	const uint8_t	x);

/** Write x in hexadecimal ASCII, four bytes, to the UART.
 */
void uart_send_hex16(	const uint16_t	x);

/**
 * Print unsigned byte \c x to uart in hexadecimal, prefixed by \c prefix.
 * A newline (CRLF) is appended.
 */
void println_hex08(
	PGM_P					prefix,
	const uint8_t	x);

/**
 * Print 16-bit unsigned integer x to uart in hexadecimal, prefixed by \c prefix.
 * A newline (CRLF) is appended.
 */
void println_hex16(
	PGM_P					prefix,
	const uint16_t	x);

/**
 * Print unsigned long \c x to uart in hexadecimal, prefixed by \c prefix.
 * A newline (CRLF) is appended.
 */
void
println_hex32(
	PGM_P						prefix,
	const uint32_t	x
);

/**
 * Print unsigned short \c x to uart, prefixed by \c prefix.
 * A newline (CRLF) is appended.
 */
void
println_u16(
	PGM_P						prefix,
	const uint16_t	x
);

/**
 * Print unsigned long \c x to uart, prefixed by \c prefix.
 * A newline (CRLF) is appended.
 */
void
println_u32(
	PGM_P						prefix,
	const uint32_t	x
);

/**
 * Print a string from the program memory to the uart.
 */
void println_P( PGM_P					s);

#if defined(__cplusplus)
}
#endif

#endif /* uart_h_ */

