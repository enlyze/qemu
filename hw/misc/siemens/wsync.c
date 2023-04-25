/*
 * QEMU educational PCI device
 *
 * Copyright (c) 2012-2015 Jiri Slaby
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qapi/visitor.h"

#define TYPE_PCI_SYNC_DEVICE "wsync"
typedef struct EduState EduState;
DECLARE_INSTANCE_CHECKER(EduState, EDU,
                         TYPE_PCI_SYNC_DEVICE)

#define OFFSET (0x61e0 + 1)
#define SIZE 0x8000

struct EduState {
    PCIDevice pdev;
    MemoryRegion mmio;
    char bytes[SIZE];
};

static uint64_t edu_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    EduState *edu = opaque;
    
    // printf("read addr=%lx size=%x\n", addr, size);

    uint64_t val = edu->bytes[addr];
    if (addr == 0x1fb) val = 1;
    if (addr == 0x4e4f) val = 1;
    
    for (int i = 0; i < strlen("cpup_interface2"); i++) {
        if (addr == 0x4e29 + i) val = "cpup_interface2"[i];
    }
    if (addr == 0x4e5d) val = 1;

    for (int i = 0; i < strlen("adn2win_2"); i++) {
        if (addr == 0x51a9 + i) val = "adn2win_2"[i];
    }
    if (addr == 0x51cf) val = 1;

    if (addr == 0x63b9) val = 0xd0;
    if (addr == 0x63ba) val = 0xc;

    if (addr == 0x6404) val = 2;

    return val;
}

static void edu_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    EduState *edu = opaque;

    // printf("write addr=%lx size=%x val=%lx\n", addr, size, val);

    edu->bytes[addr] = val;

    return;
}

static const MemoryRegionOps edu_mmio_ops = {
    .read = edu_mmio_read,
    .write = edu_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 1,
        .max_access_size = 1,
    },

};

static void pci_edu_realize(PCIDevice *pdev, Error **errp)
{
    EduState *edu = EDU(pdev);
    uint8_t *pci_conf = pdev->config;

    pci_config_set_interrupt_pin(pci_conf, 1);

    if (msi_init(pdev, 0, 1, true, false, errp)) {
        return;
    }

    int offset = pci_add_capability(pdev, 9, 0, 0x1c, errp);
    if (offset < 0) {
        return;
    }
    pci_set_quad(pdev->config + offset + 0x4, OFFSET);
    pci_set_quad(pdev->config + offset + 0xc, 0x61e0 + 2);
    // WSync
    pci_set_quad(pdev->config + offset + 0x14, 0x00636e795357);

    memory_region_init_io(&edu->mmio, OBJECT(edu), &edu_mmio_ops, edu,
                    "wsync-mmio", SIZE);
    memory_region_add_subregion(get_system_memory(), OFFSET & ~0xfff, &edu->mmio);

    memset(edu->bytes, 0, SIZE);
}

static void pci_edu_uninit(PCIDevice *pdev)
{
    msi_uninit(pdev);
}

static void edu_instance_init(Object *obj)
{
}

static void edu_class_init(ObjectClass *class, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(class);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);

    k->realize = pci_edu_realize;
    k->exit = pci_edu_uninit;
    k->vendor_id = 0x110a;
    k->device_id = 0x4051;
    k->revision = 0x10;
    k->class_id = PCI_CLASS_OTHERS;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static void pci_sync_register_types(void)
{
    static InterfaceInfo interfaces[] = {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    };
    static const TypeInfo edu_info = {
        .name          = TYPE_PCI_SYNC_DEVICE,
        .parent        = TYPE_PCI_DEVICE,
        .instance_size = sizeof(EduState),
        .instance_init = edu_instance_init,
        .class_init    = edu_class_init,
        .interfaces = interfaces,
    };

    type_register_static(&edu_info);
}
type_init(pci_sync_register_types)
