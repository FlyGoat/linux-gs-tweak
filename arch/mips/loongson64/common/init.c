/*
 * Copyright (C) 2009 Lemote Inc.
 * Author: Wu Zhangjin, wuzhangjin@gmail.com
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/bootmem.h>
#include <asm/bootinfo.h>
#include <asm/traps.h>
#include <asm/smp-ops.h>
#include <asm/cacheflush.h>
#include <asm/dma-coherence.h>

#include <loongson.h>
#include <loongson-pch.h>

#define HT_uncache_enable_reg0	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0xF0)
#define HT_uncache_base_reg0	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0xF4)
#define HT_uncache_enable_reg1	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0xF8)
#define HT_uncache_base_reg1	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0xFC)
#define HT_uncache_enable_reg2	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0x168)
#define HT_uncache_base_reg2	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0x16C)
#define HT_uncache_enable_reg3	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0x170)
#define HT_uncache_base_reg3	*(volatile unsigned int *)(loongson_sysconf.ht_control_base + 0x174)

/* Loongson CPU address windows config space base address */
unsigned long __maybe_unused _loongson_addrwincfg_base;

static void __init mips_nmi_setup(void)
{
	void *base;
	extern char except_vec_nmi;

	base = (void *)(CAC_BASE + 0x380);
	memcpy(base, &except_vec_nmi, 0x80);
	flush_icache_range((unsigned long)base, (unsigned long)base + 0x80);
}

void __init prom_init(void)
{
#ifdef CONFIG_CPU_SUPPORTS_ADDRWINCFG
	_loongson_addrwincfg_base = (unsigned long)
		ioremap(LOONGSON_ADDRWINCFG_BASE, LOONGSON_ADDRWINCFG_SIZE);
#endif

	prom_init_cmdline();
	prom_init_env();

	/* init base address of io space */
	set_io_port_base((unsigned long)
		ioremap(LOONGSON_PCIIO_BASE, LOONGSON_PCIIO_SIZE));

	if (loongson_pch->early_config)
		loongson_pch->early_config();

#ifdef CONFIG_NUMA
	prom_init_numa_memory();
#else
	prom_init_memory();
#endif

	/*init the uart base address */
	prom_init_uart_base();
	register_smp_ops(&loongson3_smp_ops);
	board_nmi_handler_setup = mips_nmi_setup;
#ifdef CONFIG_CPU_LOONGSON3
	if (!hw_coherentio) {
		/* set HT-access uncache */
		switch (loongson_sysconf.cputype) {
		case Legacy_3A:
		case Loongson_3A:
			HT_uncache_enable_reg0	= 0xc0000000; //Low 256M
			HT_uncache_base_reg0	= 0x0080fff0;
			HT_uncache_enable_reg1	= 0xc0000000; //Node 0
			HT_uncache_base_reg1	= 0x0000e000;
			HT_uncache_enable_reg2	= 0xc0100000; //Node 1
			HT_uncache_base_reg2	= 0x2000e000;
			HT_uncache_enable_reg3	= 0xc0200000; //Node 2/3
			HT_uncache_base_reg3	= 0x4000c000;
			writeq(0x0000202000000000, (void *)0x900000003ff02708);
			writeq(0xffffffe000000000, (void *)0x900000003ff02748);
			writeq(0x0000300000000086, (void *)0x900000003ff02788);
			break;
		case Legacy_3B:
		case Loongson_3B:
			HT_uncache_enable_reg0	= 0xc0000000;
			HT_uncache_base_reg0	= 0x0080fff0;
			HT_uncache_enable_reg1	= 0xc0000000;
			HT_uncache_base_reg1	= 0x00008000;
			break;
		default:
			break;
		}
	} else {
		/* set HT-access cache */
		switch (loongson_sysconf.cputype) {
		case Legacy_3A:
		case Loongson_3A:
			HT_uncache_enable_reg0	= 0x0;
			HT_uncache_enable_reg1	= 0x0;
			HT_uncache_enable_reg2	= 0x0;
			HT_uncache_enable_reg3	= 0x0;
			break;
		case Legacy_3B:
		case Loongson_3B:
			HT_uncache_enable_reg0	= 0x0;
			HT_uncache_enable_reg1	= 0x0;
			break;
		default:
			break;
		}
		printk("SET HT_DMA CACHED\n");
	}
	__sync();
#endif /* CONFIG_CPU_LOONGSON3 */
}

void __init prom_free_prom_memory(void)
{
}
