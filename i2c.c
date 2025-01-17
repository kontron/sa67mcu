// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 */

#include "i2c.h"
#include "misc.h"
#include "nvic.h"

#include "systick.h"

#define I2C_IRQ			24

#define I2C_BASE		0x400f0000
#define I2C_PWREN		(I2C_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define I2C_IIDX		(I2C_BASE + 0x1020)
#define I2C_IMASK		(I2C_BASE + 0x1028)
#define I2C_RIS			(I2C_BASE + 0x1030)
#define I2C_MIS			(I2C_BASE + 0x1038)
#define I2C_ICLR		(I2C_BASE + 0x1048)

#define INT_SSTOP		BIT(23)
#define INT_SSTART		BIT(22)
#define INT_STXFIFOTRG		BIT(19)
#define INT_SRXFIFOTRG		BIT(18)
#define INT_STXDONE		BIT(17)
#define INT_SRXDONE		BIT(16)

#define IIDX_SSTOP		0x18
#define IIDX_SSTART		0x17
#define IIDX_STXFIFOTRG		0x14
#define IIDX_SRXFIFOTRG		0x13
#define IIDX_STXDONE		0x12
#define IIDX_SRXDONE		0x11

#define I2C_CLKSEL		(I2C_BASE + 0x1004)
#define CLKSEL_MFCLK		BIT(2)

#define I2C_MSA			(I2C_BASE + 0x1210)
#define MSA_DIR			BIT(0)
#define MSA_SADDR(a)		((a) << 1)

#define I2C_MCTR		(I2C_BASE + 0x1214)
#define MCTR_MBLEN(l)		((l) << 16)
#define MCTR_ACK		BIT(3)
#define MCTR_STOP		BIT(2)
#define MCTR_START		BIT(1)
#define MCTR_BURSTRUN		BIT(0)

#define I2C_MSR			(I2C_BASE + 0x1218)
#define MSR_BUSBSY		BIT(6)
#define MSR_IDLE		BIT(5)
#define MSR_ARBLST		BIT(4)
#define MSR_DATACK		BIT(3)
#define MSR_ADRACK		BIT(2)
#define MSR_ERR			BIT(1)
#define MSR_BUSY		BIT(0)

#define I2C_MRXDATA		(I2C_BASE + 0x121c)
#define I2C_MTXDATA		(I2C_BASE + 0x1220)
#define I2C_MTPR		(I2C_BASE + 0x1224)

#define I2C_MCR			(I2C_BASE + 0x1228)
#define MCR_ACTIVE		BIT(0)

#define I2C_MFIFOSR		(I2C_BASE + 0x123c)
#define MFIFOSR_TXFIFOCNT_MASK	0xf00
#define MFIFOSR_RXFIFOCNT_MASK	0xf

#define I2C_SOAR		(I2C_BASE + 0x1250)
#define SOAR_OAREN		BIT(14)
#define SOAR_OAR(a)		((a) << 0)

#define I2C_SOAR2		(I2C_BASE + 0x1254)
#define SOAR2_OAR2_MASK(m)	((m) << 16)
#define SOAR2_OAR2EN		BIT(7)
#define SOAR2_OAR2(a)		((a) << 0)

#define I2C_SCTR		(I2C_BASE + 0x1258)
#define SCTR_TXTRIG_TXMODE	BIT(4)
#define SCTR_ACTIVE		BIT(0)

#define I2C_SRXDATA		(I2C_BASE + 0x1260)
#define I2C_STXDATA		(I2C_BASE + 0x1264)

#define I2C_SFIFOCTL		(I2C_BASE + 0x126c)
#define SFIFOCTL_RXFLUSH	BIT(15)
#define SFIFOCTL_TXFLUSH	BIT(7)

#define I2C_SFIFOSR		(I2C_BASE + 0x1270)

#ifndef CFG_I2C_ADDRESS
#define CFG_I2C_ADDRESS 0x4a
#endif

static void i2c_flush_fifos(void)
{
	iow(I2C_SFIFOCTL, SFIFOCTL_RXFLUSH | SFIFOCTL_TXFLUSH);

	/*
	 * The documention is wrong as the default value should be 0x8880, but
	 * it's actually 0x8080 (if both flush bits are set)
	 */
	while (ior(I2C_SFIFOCTL) != 0x8080);
	iow(I2C_SFIFOCTL, 0);
}

void i2c_init(void)
{
	iow(I2C_PWREN, PWREN_KEY | PWREN_ENABLE);
	iow(I2C_CLKSEL, CLKSEL_MFCLK);

	/* 100kHz clock. See Table 21-1 for controller clock settings. */
	iow(I2C_MTPR, 3);

	/* set our I2C address */
	iow(I2C_SOAR, SOAR_OAREN | SOAR_OAR(CFG_I2C_ADDRESS));

	i2c_flush_fifos();

	/* enable I2C controller mode */
	iow(I2C_MCR, MCR_ACTIVE);

	/*
	 * In target mode, no clock stretching, tx trigger mode is needed for
	 * our FSM without clock streching.
	 */
	iow(I2C_SCTR, SCTR_TXTRIG_TXMODE);

	/* unmask and enable interrupts */
	iow(I2C_IMASK, INT_SSTOP | INT_SSTART | INT_STXFIFOTRG | INT_SRXDONE);
	nvic_enable_irq(I2C_IRQ);
}

void i2c_enable_target_mode(void)
{
	/* enable I2C target mode, no */
	iow(I2C_SCTR, ior(I2C_SCTR) | SCTR_ACTIVE);
}

void i2c_disable_target_mode(void)
{
	iow(I2C_SCTR, ior(I2C_SCTR) & ~SCTR_ACTIVE);
}

static int i2c_wait_for_completion(void)
{
	unsigned int sr;

	while ((sr = ior(I2C_MSR)) & MSR_BUSY);
	if (sr & MSR_ERR)
		return -1;
	return 0;
}

int i2c_xfer_blocking(unsigned char addr,
		      unsigned char *txbuf, unsigned int txlen,
		      unsigned char *rxbuf, unsigned int rxlen)
{
	unsigned int mctr;

	if (txbuf && txlen) {
		iow(I2C_MSA, MSA_SADDR(addr));
		mctr = MCTR_MBLEN(txlen) | MCTR_START | MCTR_BURSTRUN;
		if (!rxlen)
			mctr |= MCTR_STOP;
		iow(I2C_MCTR, mctr);

		while (txlen--) {
			if (ior(I2C_MSR) & MSR_ERR)
				return -1;
			while (!(ior(I2C_MFIFOSR) & MFIFOSR_TXFIFOCNT_MASK));
			iow(I2C_MTXDATA, *(txbuf++));
		}

		if (i2c_wait_for_completion())
			return -1;
	}

	if (rxbuf && rxlen) {
		iow(I2C_MSA, MSA_SADDR(addr) | MSA_DIR);
		iow(I2C_MCTR, MCTR_MBLEN(rxlen) | MCTR_STOP | MCTR_START |
			      MCTR_BURSTRUN);

		while (rxlen--) {
			if (ior(I2C_MSR) & MSR_ERR)
				return -1;
			while (!(ior(I2C_MFIFOSR) & MFIFOSR_RXFIFOCNT_MASK));
			*(rxbuf++) = ior(I2C_MRXDATA);
		}

		if (i2c_wait_for_completion())
			return -1;
	}

	return 0;
}

enum {
	S_IDLE = 0,
	S_READ_OR_OFFSET,
	S_READ,
	S_WRITE,
};

static volatile int i2c_state = S_IDLE;
static volatile unsigned char i2c_offset;
static volatile unsigned char i2c_mem[256] = { 1, 2, 3, 4, 5, 6, 7 };

void i2c0_irq(void)
{
	unsigned int idx = ior(I2C_IIDX);

	switch (i2c_state) {
	case S_IDLE:
		switch (idx) {
		case IIDX_SSTART:
			/*
			 * Already preload the output buffer as we don't know if
			 * this is a read or a write.
			 */
			i2c_flush_fifos();
			iow(I2C_STXDATA, i2c_mem[i2c_offset]);
			i2c_state = S_READ_OR_OFFSET;
			break;
		default:
			goto unexpected;
		}
		break;
	case S_READ_OR_OFFSET:
		switch (idx) {
		case IIDX_STXFIFOTRG:
			i2c_offset += 1;
			iow(I2C_STXDATA, i2c_mem[i2c_offset]);
			i2c_state = S_READ;
			break;
		case IIDX_SRXDONE:
			i2c_offset = ior(I2C_SRXDATA);
			i2c_state = S_WRITE;
			break;
		case IIDX_SSTOP:
			i2c_state = S_IDLE;
			break;
		default:
			goto unexpected;
		}
		break;
	case S_READ:
		switch (idx) {
		case IIDX_STXFIFOTRG:
			i2c_offset += 1;
			iow(I2C_STXDATA, i2c_mem[i2c_offset]);
			break;
		case IIDX_SSTART:
			i2c_flush_fifos();
			iow(I2C_STXDATA, i2c_mem[i2c_offset]);
			i2c_state = S_READ_OR_OFFSET;
			break;
		case IIDX_SSTOP:
			i2c_state = S_IDLE;
			break;
		default:
			goto unexpected;
		}
		break;
	case S_WRITE:
		switch (idx) {
		case IIDX_SRXDONE:
			i2c_mem[i2c_offset] = ior(I2C_SRXDATA);
			i2c_offset += 1;
			break;
		case IIDX_SSTART:
			i2c_flush_fifos();
			iow(I2C_STXDATA, i2c_mem[i2c_offset]);
			i2c_state = S_READ_OR_OFFSET;
			break;
		case IIDX_SSTOP:
			i2c_state = S_IDLE;
			break;
		default:
			goto unexpected;
		}
		break;
	}

	return;

unexpected:
	printf("unexpected INT idx %d in state %d\n", idx, i2c_state);
}
