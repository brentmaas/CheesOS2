#ifndef _CHEESOS2_PCI_PCI_H
#define _CHEESOS2_PCI_PCI_H

#include <stdint.h>

#define PCI_PORT_CONFIG_ADDRESS 0xCF8
#define PCI_PORT_CONFIG_DATA 0xCFC

#define PCI_VENDOR_INVALID 0xFFFF

#define PCI_HEADER_TYPE_GENERAL 0x00
#define PCI_HEADER_TYPE_BRIDGE 0x01
#define PCI_HEADER_TYPE_CARDBUS 0x02

#define PCI_HEADER_TYPE_MASK 0x3
#define PCI_HEADER_TYPE_MULTI_FUNCTIONAL_BIT (1 << 6)

enum pci_offset {
    PCI_OFFSET_DEVICE_ID = 0x00, // u16
    PCI_OFFSET_VENDOR_ID = 0x02, // u16
    PCI_OFFSET_STATUS = 0x04, // u16
    PCI_OFFSET_COMMAND = 0x06, // u16
    PCI_OFFSET_CLASS = 0x08, // u8
    PCI_OFFSET_SUBCLASS = 0x09, // u8
    PCI_OFFSET_PROG_IF = 0x0A, // u8
    PCI_OFFSET_REVISION_ID = 0x0B, // u8
    PCI_OFFSET_BIST = 0x0C, // u8
    PCI_OFFSET_HEADER_TYPE = 0x0D, // enum pci_header_type as u8
    PCI_OFFSET_LATENCY_TIMER = 0x0E, // u8
    PCI_OFFSET_CACHE_LINE_SIZE = 0x0F, // u8

    PCI_OFFSET_BAR0 = 0x10, // u32
    PCI_OFFSET_BAR1 = 0x14, // u32
    PCI_OFFSET_BAR2 = 0x18, // u32
    PCI_OFFSET_BAR3 = 0x1C, // u32
    PCI_OFFSET_BAR4 = 0x20, // u32
    PCI_OFFSET_BAR5 = 0x24 // u32
};

struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
};

void pci_config_write8(struct pci_device device, enum pci_offset offset, uint8_t value);
void pci_config_write16(struct pci_device device, enum pci_offset offset, uint16_t value);
void pci_config_write32(struct pci_device device, enum pci_offset offset, uint32_t value);

uint8_t pci_config_read8(struct pci_device device, enum pci_offset offset);
uint16_t pci_config_read16(struct pci_device device, enum pci_offset offset);
uint32_t pci_config_read32(struct pci_device device, enum pci_offset offset);

typedef void (*pci_scan_cbk)(struct pci_device device, uint16_t vendor_id);

void pci_scan(pci_scan_cbk callback);

#endif
