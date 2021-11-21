// vim: ts=4 shiftwidth=4
#ifndef DAC8560_h_
#define DAC8560_h_

/** \file
 * DAC8560 interface. This is specific to SCALP03G.
 *
 * The physical interface is hardware SPI, chip select is PB4, active high.
 *
 * Software can be configured to either offload data transfer to SPI transfer complete interrupt
 * or send it directly.
 *
 * Test results for 1024 DAC settings with cpu clock 10MHz, SPI clock 5MHz:
 * With IRQ-s: 22.7 ms
 * Without IRQ-s: 10.24 ms, thus one transmit takes 100 clock cycles or 10uS. At 1kHz repetition
 * rate it consumes 1% of CPU. Not bad.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef enum {
	/** Offload data transfer to SPI transfer complete interrupt. */
	DAC8560_WITH_IRQ,
	/** Do not offload data transfer. */
	DAC8560_WITHOUT_IRQ
} DAC8560_IRQ_USAGE;

/** Initialize DAC port as follows: PB4,PB5,PB7 output.
 * Init SPI to Master, F_CPU/4, MSB first, SCK is low when idle, leading edge is setup.
 *
 * \param[in]	dac8560_irq_usage	Whether to offload the data transfer to interrupts.
 */
void
DAC8560_Init(
	const DAC8560_IRQ_USAGE	dac8560_irq_usage
);

/** Write 16-bit value to DAC unless DAC is busy (see DAC8560_Busy).
 *
 * If data transfer is offloaded to interrupts, requires interrupt to be enabled.
 *
 * \param[in]	Data	16-bit value to be sent to DAC.
 */
void
DAC8560_Write(
	const uint16_t	Data
);

/** Is value conversion in progress (i.e. SPI transfer)?
 *
 * Always is false when not using IRQ-s.
 */
bool
DAC8560_Busy(void);

#if defined(__cplusplus)
}
#endif

#endif /* DAC8560_h_ */

