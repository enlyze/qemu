/*
 *  ioapic.c IOAPIC emulation logic
 *
 *  Copyright (c) 2011 Jan Kiszka, Siemens AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hw/sysbus.h"

#ifndef HW_INTC_IOAPIC_H
#define HW_INTC_IOAPIC_H

#define IOAPIC_NUM_PINS 24
#define IO_APIC_DEFAULT_ADDRESS 0xfec00000
#define IO_APIC_SECONDARY_ADDRESS (IO_APIC_DEFAULT_ADDRESS + 0x10000)
#define IO_APIC_SECONDARY_IRQBASE 24 /* primary 0 -> 23, secondary 24 -> 47 */

#define TYPE_KVM_IOAPIC "kvm-ioapic"
#define TYPE_IOAPIC "ioapic"

void ioapic_eoi_broadcast(int vector);

#define MAX_IOAPICS                     2

#define IOAPIC_IOREGSEL                 0x00
#define IOAPIC_IOWIN                    0x10
#define IOAPIC_EOI                      0x40

struct IOAPICCommonState {
    SysBusDevice busdev;
    MemoryRegion io_memory;
    uint8_t id;
    uint8_t ioregsel;
    uint32_t irr;
    uint64_t ioredtbl[IOAPIC_NUM_PINS];
    Notifier machine_done;
    uint8_t version;
    uint64_t irq_count[IOAPIC_NUM_PINS];
    int irq_level[IOAPIC_NUM_PINS];
    int irq_eoi[IOAPIC_NUM_PINS];
    QEMUTimer *delayed_ioapic_service_timer;
};

extern struct IOAPICCommonState *ioapics[MAX_IOAPICS];
uint64_t ioapic_mem_read(void *opaque, hwaddr addr, unsigned int size);
void ioapic_mem_write(void *opaque, hwaddr addr, uint64_t val, unsigned int size);

#endif /* HW_INTC_IOAPIC_H */
