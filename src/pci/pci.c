#include "pci/pci.h"
#include "core/io.h"

#define PCI_MAKE_ADDRESS(device, offset) \
    (0x80000000 | ((device).bus << 16) | ((device).slot << 11) | ((device).function << 8) | offset)

void pci_config_write8(struct pci_device device, enum pci_offset offset, uint8_t value) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    io_out8(PCI_PORT_CONFIG_DATA + (offset & 0x3), value);
}

void pci_config_write16(struct pci_device device, enum pci_offset offset, uint16_t value) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    io_out16(PCI_PORT_CONFIG_DATA + (offset & 0x2), value);
}

void pci_config_write32(struct pci_device device, enum pci_offset offset, uint32_t value) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    io_out32(PCI_PORT_CONFIG_DATA, value);
}

uint8_t pci_config_read8(struct pci_device device, enum pci_offset offset) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    return io_in8(PCI_PORT_CONFIG_DATA + (offset & 0x3));
}

uint16_t pci_config_read16(struct pci_device device, enum pci_offset offset) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    return io_in16(PCI_PORT_CONFIG_DATA + (offset & 0x2));
}

uint32_t pci_config_read32(struct pci_device device, enum pci_offset offset) {
    io_out32(PCI_PORT_CONFIG_ADDRESS, PCI_MAKE_ADDRESS(device, offset));
    return io_in32(PCI_PORT_CONFIG_DATA);
}

static void pci_scan_slot(pci_scan_cbk callback, uint8_t bus, uint8_t slot) {
    struct pci_device device = {bus, slot, 0};
    uint16_t vendor_id = pci_config_read16(device, PCI_OFFSET_VENDOR_ID);
    if (vendor_id == PCI_VENDOR_INVALID) {
        return;
    }

    callback(device, vendor_id);

    const uint8_t header_type = pci_config_read8(device, PCI_OFFSET_HEADER_TYPE);
    if ((header_type & PCI_HEADER_TYPE_MULTI_FUNCTIONAL_BIT) != 0) {
        for (device.function = 1; device.function < 8; ++device.function) {
            vendor_id = pci_config_read16(device, PCI_OFFSET_VENDOR_ID);
            if (vendor_id != PCI_VENDOR_INVALID) {
                callback(device, vendor_id);
            }
        }
    }
}

bool pci_probe_mech1() {
    uint32_t orig_val = io_in32(PCI_PORT_CONFIG_ADDRESS);
    io_out32(PCI_PORT_CONFIG_ADDRESS, 0x80000000);
    bool has_mech1 = io_in32(PCI_PORT_CONFIG_ADDRESS) == 0x80000000;
    io_out32(PCI_PORT_CONFIG_ADDRESS, orig_val);
    return has_mech1;
}

bool pci_probe_mech2() {
    uint8_t orig_val = io_in8(PCI_PORT_CONFIG_ADDRESS);
    io_out8(PCI_PORT_CONFIG_ADDRESS, 0);
    bool has_mech2_lower = io_in8(PCI_PORT_CONFIG_ADDRESS) == 0;
    io_out8(PCI_PORT_CONFIG_ADDRESS, orig_val);

    if (!has_mech2_lower) {
        return false;
    }

    orig_val = io_in8(PCI_PORT_CONFIG_ADDRESS + 4);
    bool has_mech2 = io_in8(PCI_PORT_CONFIG_ADDRESS + 4) == 0;
    io_out8(PCI_PORT_CONFIG_ADDRESS + 4, orig_val);
    return has_mech2;
}

void pci_scan(pci_scan_cbk callback) {
    uint8_t bus = 255;
    do {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            pci_scan_slot(callback, bus, slot);
        }
    } while (bus-- != 0);
}
