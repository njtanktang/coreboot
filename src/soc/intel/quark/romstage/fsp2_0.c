/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2016 Intel Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch/early_variables.h>
#include <console/console.h>
#include <cbfs.h>
#include <cbmem.h>
#include "../chip.h"
#include <cpu/x86/cache.h>
#include <fsp/util.h>
#include <soc/pci_devs.h>
#include <soc/pm.h>
#include <soc/romstage.h>
#include <soc/reg_access.h>

asmlinkage void *car_stage_c_entry(void)
{
	struct postcar_frame pcf;
	bool s3wake;
	uintptr_t top_of_ram;
	uintptr_t top_of_low_usable_memory;

	post_code(0x20);
	console_init();

	/* Initialize DRAM */
	s3wake = fill_power_state() == ACPI_S3;
	fsp_memory_init(s3wake);

	/* Disable the ROM shadow 0x000e0000 - 0x000fffff */
	disable_rom_shadow();

	/* Initialize the PCIe bridges */
	pcie_init();

	if (postcar_frame_init(&pcf, 1*KiB))
		die("Unable to initialize postcar frame.\n");

	/* Locate the top of RAM */
	top_of_low_usable_memory = (uintptr_t) cbmem_top();
	top_of_ram = ALIGN(top_of_low_usable_memory, 16 * MiB);

	/* Cache postcar and ramstage */
	postcar_frame_add_mtrr(&pcf, top_of_ram - (16 * MiB), 16 * MiB,
		MTRR_TYPE_WRBACK);

	/* Cache RMU area */
	postcar_frame_add_mtrr(&pcf, (uintptr_t) top_of_low_usable_memory,
		0x10000, MTRR_TYPE_WRTHROUGH);

	/* Cache ESRAM */
	postcar_frame_add_mtrr(&pcf, 0x80000000, 0x80000, MTRR_TYPE_WRBACK);

	/* Cache SPI flash - Write protect not supported */
	postcar_frame_add_mtrr(&pcf, (uint32_t)(-CONFIG_ROM_SIZE),
		CONFIG_ROM_SIZE, MTRR_TYPE_WRTHROUGH);

	run_postcar_phase(&pcf);
	return NULL;
}

static struct chipset_power_state power_state CAR_GLOBAL;

struct chipset_power_state *get_power_state(void)
{
	return (struct chipset_power_state *)car_get_var_ptr(&power_state);
}

int fill_power_state(void)
{
	struct chipset_power_state *ps = get_power_state();

	ps->prev_sleep_state = 0;
	printk(BIOS_SPEW, "prev_sleep_state %d\n", ps->prev_sleep_state);
	return ps->prev_sleep_state;
}

void platform_fsp_memory_init_params_cb(FSPM_UPD *fspm_upd)
{
	FSPM_ARCH_UPD *aupd;
	const struct device *dev;
	const struct soc_intel_quark_config *config;
	char *rmu_file;
	size_t rmu_file_len;
	FSP_M_CONFIG *upd;

	/* Clear SMI and wake events */
	clear_smi_and_wake_events();

	/* Locate the RMU data file in flash */
	rmu_file = cbfs_boot_map_with_leak("rmu.bin", CBFS_TYPE_RAW,
		&rmu_file_len);
	if (!rmu_file)
		die("Microcode file (rmu.bin) not found.");

	/* Locate the configuration data from devicetree.cb */
	dev = dev_find_slot(0, LPC_DEV_FUNC);
	if (!dev) {
		die("ERROR - LPC device not found!");
	}
	config = dev->chip_info;

	/* Update the architectural UPD values. */
	aupd = &fspm_upd->FspmArchUpd;
	aupd->BootLoaderTolumSize = cbmem_overhead_size();
	aupd->StackBase = (void *)(CONFIG_FSP_ESRAM_LOC - aupd->StackSize);
	aupd->BootMode = FSP_BOOT_WITH_FULL_CONFIGURATION;

	/* Display the ESRAM layout */
	if (IS_ENABLED(CONFIG_DISPLAY_ESRAM_LAYOUT)) {
		printk(BIOS_SPEW, "\nESRAM Layout:\n\n");
		printk(BIOS_SPEW,
			"+-------------------+ 0x80080000 - ESRAM end\n");
		printk(BIOS_SPEW, "| FSP binary        |\n");
		printk(BIOS_SPEW,
			"+-------------------+ 0x%08x (CONFIG_FSP_ESRAM_LOC)\n",
			CONFIG_FSP_ESRAM_LOC);
		printk(BIOS_SPEW, "| FSP stack         |\n");
		printk(BIOS_SPEW, "+-------------------+ 0x%p\n",
			aupd->StackBase);
		printk(BIOS_SPEW, "|                   |\n");
		printk(BIOS_SPEW, "+-------------------+ 0x%p\n",
			_car_relocatable_data_end);
		printk(BIOS_SPEW, "| coreboot data     |\n");
		printk(BIOS_SPEW, "+-------------------+ 0x%p\n",
			_car_stack_end);
		printk(BIOS_SPEW, "| coreboot stack    |\n");
		printk(BIOS_SPEW,
			"+-------------------+ 0x80000000 - ESRAM start\n\n");
	}

	/* Update the UPD data for MemoryInit */
	upd = &fspm_upd->FspmConfig;
	upd->AddrMode = config->AddrMode;
	upd->ChanMask = config->ChanMask;
	upd->ChanWidth = config->ChanWidth;
	upd->DramDensity = config->DramDensity;
	upd->DramRonVal = config->DramRonVal;
	upd->DramRttNomVal = config->DramRttNomVal;
	upd->DramRttWrVal = config->DramRttWrVal;
	upd->DramSpeed = config->DramSpeed;
	upd->DramType = config->DramType;
	upd->DramWidth = config->DramWidth;
	upd->EccScrubBlkSize = config->EccScrubBlkSize;
	upd->EccScrubInterval = config->EccScrubInterval;
	upd->Flags = config->Flags;
	upd->FspReservedMemoryLength = config->FspReservedMemoryLength;
	upd->RankMask = config->RankMask;
	upd->RmuBaseAddress = (uintptr_t)rmu_file;
	upd->RmuLength = rmu_file_len;
	upd->SerialPortWriteChar = console_log_level(BIOS_SPEW)
		? (uintptr_t)fsp_write_line : 0;
	upd->SmmTsegSize = IS_ENABLED(CONFIG_HAVE_SMI_HANDLER) ?
		config->SmmTsegSize : 0;
	upd->SocRdOdtVal = config->SocRdOdtVal;
	upd->SocWrRonVal = config->SocWrRonVal;
	upd->SocWrSlewRate = config->SocWrSlewRate;
	upd->SrInt = config->SrInt;
	upd->SrTemp = config->SrTemp;
	upd->tCL = config->tCL;
	upd->tFAW = config->tFAW;
	upd->tRAS = config->tRAS;
	upd->tRRD = config->tRRD;
	upd->tWTR = config->tWTR;
}
