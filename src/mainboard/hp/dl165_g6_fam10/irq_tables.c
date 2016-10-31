#ifdef GETPIR
#include "pirq_routing.h"
#else
#include <arch/pirq_routing.h>
#endif

static const struct irq_routing_table intel_irq_routing_table = {
	PIRQ_SIGNATURE,		/* u32 signature */
	PIRQ_VERSION,		/* u16 version */
	32 + 16 * 11,		/* Max. number of devices on the bus */
	0x00,			/* Interrupt router bus */
	(0x02 << 3) | 0x0,	/* Interrupt router dev */
	0,			/* IRQs devoted exclusively to PCI usage */
	0x1166,			/* Vendor */
	0x36,			/* Device */
	0,			/* Miniport */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* u8 rfu[11] */
	0xe9,			/* Checksum (has to be set to some value that
				 * would give 0 after the sum of all bytes
				 * for this structure (including checksum).
				 */
	{
		/* bus,        dev | fn,   {link, bitmap}, {link, bitmap}, {link, bitmap}, {link, bitmap}, slot, rfu */
		{0x01, (0x0e << 3) | 0x0, {{0x07, 0x0020}, {0x07, 0x0020}, {0x07, 0x0020}, {0x07, 0x0020}}, 0x0, 0x0}, /* 1166:024a */
		{0x00, (0x03 << 3) | 0x0, {{0x01, 0x0400}, {0x01, 0x0400}, {0x01, 0x0400}, {0x01, 0x0400}}, 0x0, 0x0}, /* 1166:0223 */
		{0x00, (0x06 << 3) | 0x0, {{0x24, 0xdac0}, {0x24, 0xdac0}, {0x24, 0xdac0}, {0x24, 0xdac0}}, 0x0, 0x0}, /* 1166:0140 */
		{0x00, (0x07 << 3) | 0x0, {{0x23, 0xdac0}, {0x23, 0xdac0}, {0x23, 0xdac0}, {0x23, 0xdac0}}, 0x0, 0x0}, /* 1166:0142 */
		{0x00, (0x08 << 3) | 0x0, {{0x22, 0xdac0}, {0x22, 0xdac0}, {0x22, 0xdac0}, {0x22, 0xdac0}}, 0x0, 0x0}, /* 1166:0144 */
		{0x00, (0x09 << 3) | 0x0, {{0x21, 0xdac0}, {0x21, 0xdac0}, {0x21, 0xdac0}, {0x21, 0xdac0}}, 0x0, 0x0}, /* 1166:0142 */
		{0x00, (0x0a << 3) | 0x0, {{0x20, 0xdac0}, {0x20, 0xdac0}, {0x20, 0xdac0}, {0x20, 0xdac0}}, 0x0, 0x0}, /* 1166:0144 */
		{0x02, (0x02 << 3) | 0x0, {{0x28, 0xdac0}, {0x27, 0xdac0}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x0, 0x0}, /* 14e4:1648 */
		{0x06, (0x00 << 3) | 0x0, {{0x21, 0xdac0}, {0x21, 0xdac0}, {0x21, 0xdac0}, {0x21, 0xdac0}}, 0x2, 0x0},
		{0x03, (0x00 << 3) | 0x0, {{0x24, 0xdac0}, {0x24, 0xdac0}, {0x24, 0xdac0}, {0x24, 0xdac0}}, 0x2, 0x0},
		{0x07, (0x00 << 3) | 0x0, {{0x2a, 0xdac0}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x0, 0x0}, /* 102b:0522 */
	}
};


unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr, &intel_irq_routing_table);
}
