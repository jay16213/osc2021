#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H

#define PERIPHERAL_BASE (0x3F000000)
#define AUX_MU_IO_REG   (PERIPHERAL_BASE+0x00215040)
#define AUX_MU_LSR_REG  (PERIPHERAL_BASE+0x00215054)
#define AUXENB          (PERIPHERAL_BASE+0x00215004)
#define AUX_MU_CNTL_REG (PERIPHERAL_BASE+0x00215060)
#define AUX_MU_IER_REG  (PERIPHERAL_BASE+0x00215044)
#define AUX_MU_LCR_REG  (PERIPHERAL_BASE+0x0021504C)
#define AUX_MU_MCR_REG  (PERIPHERAL_BASE+0x00215050)
#define AUX_MU_BAUD_REG (PERIPHERAL_BASE+0x00215068)
#define AUX_MU_IIR_REG  (PERIPHERAL_BASE+0x00215048)

#define GPFSEL1         (PERIPHERAL_BASE+0x00200004)
#define GPPUD           (PERIPHERAL_BASE+0x00200094)
#define GPPUDCLK0       (PERIPHERAL_BASE+0x00200098)
#define GPPUDCLK1       (PERIPHERAL_BASE+0x0020009C)

#endif