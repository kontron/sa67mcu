int main(void);

extern unsigned int _etext;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int _sbss;
extern unsigned int _ebss;
extern unsigned int _estack;

static void default_handler(void)
{
	while (1);
}

void reset_handler(void)
{
	unsigned int *src, *dst;

	/* copy data section */
	src = &_etext;
	dst = &_sdata;
	while (dst < &_edata)
		*dst++ = *src++;

	/* clear bss */
	dst = &_sbss;
	while (dst < &_ebss)
		*dst++ = 0;

	main();

	while (1);
}

void nmi_handler(void) __attribute__((weak, alias("default_handler")));
void hardfault_handler(void) __attribute__((weak, alias("default_handler")));
void svc_handler(void) __attribute__((weak, alias("default_handler")));
void pendsv_handler(void) __attribute__((weak, alias("default_handler")));
void systick_handler(void) __attribute__((weak, alias("default_handler")));
void group0_irq(void) __attribute__((weak, alias("default_handler")));
void group1_irq(void) __attribute__((weak, alias("default_handler")));
void timg1_irq(void) __attribute__((weak, alias("default_handler")));
void adc0_irq(void) __attribute__((weak, alias("default_handler")));
void spi0_irq(void) __attribute__((weak, alias("default_handler")));
void uart1_irq(void) __attribute__((weak, alias("default_handler")));
void uart0_irq(void) __attribute__((weak, alias("default_handler")));
void timg0_irq(void) __attribute__((weak, alias("default_handler")));
void timg2_irq(void) __attribute__((weak, alias("default_handler")));
void timg4_irq(void) __attribute__((weak, alias("default_handler")));
void i2c0_irq(void) __attribute__((weak, alias("default_handler")));
void i2c1_irq(void) __attribute__((weak, alias("default_handler")));
void dma_irq(void) __attribute__((weak, alias("default_handler")));

void *vector_table[] __attribute__ ((section (".vectors"))) = {
	&_estack,
	reset_handler,
	nmi_handler,
	hardfault_handler,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	svc_handler,
	0,
	0,
	pendsv_handler,
	systick_handler,
	group0_irq,
	group1_irq,
	timg1_irq,
	0,
	adc0_irq,
	0,
	0,
	0,
	0,
	spi0_irq,
	0,
	0,
	0,
	uart1_irq,
	0,
	uart0_irq,
	timg0_irq,
	0,
	timg2_irq,
	0,
	timg4_irq,
	0,
	0,
	0,
	i2c0_irq,
	i2c1_irq,
	0,
	0,
	0,
	0,
	0,
	dma_irq,
};
