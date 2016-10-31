/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2010 Joseph Smith <joe@settoplinux.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef NORTHBRIDGE_INTEL_I82830_RAMINIT_H
#define NORTHBRIDGE_INTEL_I82830_RAMINIT_H

/* 82830 Northbridge PCI device */
#define NORTHBRIDGE	PCI_DEV(0, 0, 0)

/* The 82830 supports max. 2 dual-sided SO-DIMMs. */
#define DIMM_SOCKETS	2

#endif /* NORTHBRIDGE_INTEL_I82830_RAMINIT_H */
