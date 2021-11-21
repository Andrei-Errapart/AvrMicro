/*
vim: ts=2
vim: shiftwidth=2
*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Micro/CBuffer.h>
#include <Micro/uart.h>

#if defined (__AVR_ATmega644__)
/** Internal use only: Use new USART interface? */
#define	MICRO_NEW_INTERFACE
#endif

#if defined(UART_RS485_PORT)
# if !defined(UART_RS485_PIN)
#error Both UART_RS485_PORT and UART_RS485_PIN should be defined.
# endif
#else
#	if defined(UART_RS485_PIN)
#error Neither UART_RS485_PORT nor UART_RS485_PIN should be defined.
# endif
#endif
/*****************************************************************************/
void
uart_setup(
	const uint16_t	baud_rate_divisor
)
{
#if defined(MICRO_NEW_INTERFACE)
	UBRR0	= baud_rate_divisor;
	UCSR0A = 0x00;
	UCSR0B = (1<<RXEN0) | (1<<RXCIE0) | (1<<TXEN0) | (1<<UDRIE0)
#if defined(UART_RS485_PORT) && defined(UART_RS485_PIN)
		| _BV(TXCIE0)
#endif
		;
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* Frame format: 8N1 */
#else
	UBRRH = baud_rate_divisor >> 8;
	UBRRL = baud_rate_divisor & 0xFF;
	UCSRB = _BV(RXEN) | _BV(RXCIE) | _BV(TXEN) | _BV(UDRIE)
#if defined(UART_RS485_PORT) && defined(UART_RS485_PIN)
		| _BV(TXCIE)
#endif
		;
	UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
#endif
}

/*****************************************************************************/
void
uart_close()
{
#if defined(MICRO_NEW_INTERFACE)
	UCSR0B = 0x00;
#else
	UCSRC = 0x00;
#endif
}

/*****************************************************************************/
/** Byte received on UART. */
#if defined(MICRO_NEW_INTERFACE)
SIGNAL(SIG_USART_RECV)
{
	for (;;) {
		/* Check flags. */
		const uint8_t	flags = UCSR0A;
		if (flags & (1<<RXC0)) {
			/* Read byte. */
			const uint8_t	udr = UDR0;
			if (uart_read_callback) {
				uart_read_callback(udr);
			}
		} else {
			/* Stop. */
			break;
		}
	}
}
#else
ISR(USART_RXC_vect)
{
	for (;;) {
		/* Check flags. */
		const uint8_t	flags = UCSRA;
		if (flags & _BV(RXC)) {
			/* Read byte. */
			const uint8_t	udr = UDR;
			if (uart_read_callback) {
				uart_read_callback(udr);
			}
		} else {
			/* Stop. */
			break;
		}
	}
}
#endif

/*****************************************************************************/
static CBuffer<uint8_t, 128>	tx_buffer;

/*****************************************************************************/
/** Data register empty: more work to do. */
#if defined(MICRO_NEW_INTERFACE)
SIGNAL(SIG_USART_DATA)
{
	uint8_t	txchar;
	if (tx_buffer.Pop(txchar)) {
#if defined(UART_RS485_PORT) && defined(UART_RS485_PIN)
		UART_RS485_PORT |= _BV(UART_RS485_PIN);
#endif
		UDR0 = txchar;
	} else {
		// Disable DataRegisterEmpty interrupt.
		UCSR0B &= ~(1<<UDRIE0);
	}
}
#else
ISR(USART_UDRE_vect)
{
	uint8_t	txchar;
	if (tx_buffer.Pop(txchar)) {
#if defined(UART_RS485_PORT) && defined(UART_RS485_PIN)
		UART_RS485_PORT |= _BV(UART_RS485_PIN);
#endif
		UDR = txchar;
	} else {
		// Disable DataRegisterEmpty interrupt.
		UCSRB &= ~_BV(UDRIE);
	}
}
#endif

/*****************************************************************************/
#if defined(UART_RS485_PORT) && defined(UART_RS485_PIN)
#if defined(MICRO_NEW_INTERFACE)
SIGNAL(SIG_USART_TRANS)
#else
ISR(USART_TXC_vect)
#endif // MICRO_NEW_INTERFACE
{
	if (tx_buffer.IsEmpty())
	{
		UART_RS485_PORT &= ~_BV(UART_RS485_PIN);
	}
}
#endif // RS485

/*****************************************************************************/
#if defined(MICRO_NEW_INTERFACE)
#define	ENABLE_DATA_REGISTER_EMPTY() do { UCSR0B |= _BV(UDRIE0); } while(0)
#else
#define	ENABLE_DATA_REGISTER_EMPTY() do { UCSRB |= _BV(UDRIE); } while(0)
#endif

/**
 * Print character to the uart.
 */
void
uart_putchar(	const uint8_t	c)
{
	tx_buffer.Push(c);
	ENABLE_DATA_REGISTER_EMPTY();
}

/*****************************************************************************/
void
uart_send_P(	PGM_P	s)
{
	for (;; ++s) {
		const uint8_t	c = pgm_read_byte(s);
		if (c==0) {
			break;
		} else {
			tx_buffer.Push(c);
		}
	}

	ENABLE_DATA_REGISTER_EMPTY();
}

/*****************************************************************************/
void
uart_send(	const char*	s)
{
	for (;; ++s) {
		const uint8_t	c = *s;
		if (c==0) {
			break;
		} else {
			tx_buffer.Push(c);
		}
	}

	ENABLE_DATA_REGISTER_EMPTY();
}

/*****************************************************************************/
void
uart_send_crlf()
{
	uart_send_P(PSTR("\r\n"));
}

/*****************************************************************************/
static const char	hexchars[]		PROGMEM	= "0123456789ABCDEF";

/*****************************************************************************/
void
uart_send_hex08(	const uint8_t	x)
{
	uart_putchar(pgm_read_byte(hexchars + ((x >>  4) & 0x0F)));
	uart_putchar(pgm_read_byte(hexchars + ((x >>  0) & 0x0F)));
}

/*****************************************************************************/
void
uart_send_hex16(	const uint16_t	x)
{
	uart_putchar(pgm_read_byte(hexchars + ((x >> 12) & 0x0F)));
	uart_putchar(pgm_read_byte(hexchars + ((x >>  8) & 0x0F)));
	uart_putchar(pgm_read_byte(hexchars + ((x >>  4) & 0x0F)));
	uart_putchar(pgm_read_byte(hexchars + ((x >>  0) & 0x0F)));
}

/*****************************************************************************/
static void
uart_send_u16(	const uint16_t	x)
{
	char		buffer[6];
	uint8_t	i;

	for (i=0; i<sizeof(buffer); ++i) {
		buffer[i] = 0;
	}
	utoa(x, buffer, 10);
	for (i=0; i<sizeof(buffer) && buffer[i]!=0; ++i) {
		uart_putchar(buffer[i]);
	}
}

/*****************************************************************************/
void println_hex08(
	PGM_P					prefix,
	const uint8_t	x)
{
	uart_send_P(prefix);
	uart_putchar(':');
	uart_send_hex08(x);
	uart_send_crlf();
}

/*****************************************************************************/
void println_hex16(
	PGM_P					prefix,
	const uint16_t	x)
{
	uart_send_P(prefix);
	uart_putchar(':');
	uart_send_hex16(x);
	uart_send_crlf();
}

/*****************************************************************************/
void
println_hex32(
	PGM_P						prefix,
	const uint32_t	x
)
{
	uart_send_P(prefix);
	uart_putchar(':');
	uart_send_hex16(x >> 16);
	uart_send_hex16(x);
	uart_send_crlf();
}

/*****************************************************************************/
void println_u16(
	PGM_P						prefix,
	const uint16_t	x)
{
	uart_send_P(prefix);
	uart_putchar(':');
	uart_send_u16(x);
	uart_send_crlf();
}

/*****************************************************************************/
void
println_u32(
	PGM_P						prefix,
	const uint32_t	x
)
{
	char		buffer[13];
	uint8_t	i;
	uart_send_P(prefix);
	uart_putchar(':');
	ultoa(x, buffer, 10);
	for (i=0; i<sizeof(buffer) && buffer[i]!=0; ++i) {
		uart_putchar(buffer[i]);
	}
	uart_send_crlf();
}

/*****************************************************************************/
void println_P( PGM_P					s)
{
	uart_send_P(s);
	uart_send_crlf();
}

