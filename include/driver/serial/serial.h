#ifndef _CHEESOS2_DRIVER_SERIAL_SERIAL_H
#define _CHEESOS2_DRIVER_SERIAL_SERIAL_H

#include <stdint.h>

// Default serial ports. According to https://wiki.osdev.org/Serial_Ports,
// these two port addresses are relatively safe to use, but the other two
// listed there are not so reliable, so they are omitted.
#define SERIAL_PORT_1 (0x3F8)
#define SERIAL_PORT_2 (0x2F8)

// Serial register offsets. Add these to a SERIAL_PORT_X to get the
// final port address. Modem register offsets are included, but no
// detailed definition of its port is included.

// Data port: Reading gets the next byte from the input buffer,
// writing writes a byte to the output buffer. Only available when divisor_latch_access = 0.
#define SERIAL_REG_DATA (0)

// Serial clock divisor. Writing a value to these two ports sets the divisor
// of the serial clock, the final baud rate is 115200 divided by this value.
// 0 is UB, only available when divisor_latch_access = 1.
#define SERIAL_REG_DIVISOR_LOW (0)
#define SERIAL_REG_DIVISOR_HIGH (1)

// Interrupt enable register. Bits written to this register enable
// various interrupts related to the serial port. See `serial_reg_enable_int`.
// Only available when divisor_latch_access = 0.
#define SERIAL_REG_ENABLE_INT (1)

// Interrupt identification register. Returns status of the currently pending interrupt.
// See `serial_reg_int_ident`. Read only.
#define SERIAL_REG_INT_IDENT (2)

// FIFO control register. See `serial_reg_fifo_control`. Write-only.
#define SERIAL_REG_FIFO_CONTROL (2)

// Used to configure serial line properties such as parity, stop bits, and so on.
// See serial_reg_line_control.
#define SERIAL_REG_LINE_CONTROL (3)

// Contains status information about the modem.
#define SERIAL_REG_MODEM_CONTROL (4)

// Contains status information about the line. See serial_reg_line_status.
#define SERIAL_REG_LINE_STATUS (5)

// Used to control the MODEM.
#define SERIAL_REG_MODEM_STATUS (6)
#define SERIAL_REG_SCRATCHPAD (7)

typedef struct __attribute__((packed)) {
    uint8_t enable_data_available_int : 1;
    uint8_t enable_tx_holding_empty_int : 1;
    uint8_t enable_line_status_int : 1;
    uint8_t enable_model_status_int : 1;
    uint8_t : 4;
} serial_reg_enable_int;

typedef enum {
    SERIAL_INT_TYPE_MODEM_STATUS = 0x0,
    // Transmitter holding register is empty, indicating a byte can be written again.
    SERIAL_INT_TYPE_TX_HOLDING_EMPTY = 0x1,
    // Input data available or trigger level reached.
    SERIAL_INT_TYPE_DATA_AVAILABLE = 0x2,
    // Overrun, parity or framing error, or break interrupt.
    SERIAL_INT_TYPE_LINE_STATUS = 0x3
} serial_int_type;

typedef struct __attribute__((packed)) {
    // set 0 when an interrupt is pending.
    uint8_t no_int_pending : 1;
    serial_int_type highest_pending : 2;
    // set to 1 (along with SERIAL_INT_TYPE_DATA_AVAILABLE) when a
    // timeout interrupt is pending in fifo mode.
    uint8_t fifo_timeout_pending : 1;
    uint8_t : 2;
    // Set to 0x11 when fifo is enabled.
    uint8_t fifo_enabled : 2;
} serial_reg_int_ident;

typedef enum {
    SERIAL_TRIGGER_LEVEL_1_BYTE = 0x0,
    SERIAL_TRIGGER_LEVEL_4_BYTES = 0x1,
    SERIAL_TRIGGER_LEVEL_8_BYTES = 0x2,
    SERIAL_TRIGGER_LEVEL_14_BYTES = 0x3
} serial_trigger_level;

typedef struct __attribute__((packed)) {
    uint8_t fifo_enable : 1;
    uint8_t clear_rx_buffer : 1;
    uint8_t clear_tx_buffer : 1;
    uint8_t : 3;
    serial_trigger_level trigger_level : 2;
} serial_reg_fifo_control;

typedef enum {
    SERIAL_DATA_SIZE_5_BITS = 0,
    SERIAL_DATA_SIZE_6_BITS = 1,
    SERIAL_DATA_SIZE_7_BITS = 2,
    SERIAL_DATA_SIZE_8_BITS = 3
} serial_data_size;

typedef enum {
    SERIAL_STOP_BITS_ONE = 0,
    SERIAL_STOP_BITS_TWO = 1
} serial_stop_bits;

typedef struct __attribute__((packed)) {
    serial_data_size data_size : 2;
    serial_stop_bits stop_bits : 1;
    uint8_t enable_parity : 1;
    uint8_t enable_even_odd_parity : 1;
    uint8_t enable_stick_parity : 1;
    uint8_t enable_break : 1;
    uint8_t divisor_latch_access : 1;
} serial_reg_line_control;

typedef struct __attribute__((packed)) {
    uint8_t data_available : 1;
    uint8_t overrun_error : 1;
    uint8_t parity_error : 1;
    uint8_t framing_error : 1;
    uint8_t break_int : 1;
    uint8_t tx_holding_empty : 1;
    uint8_t tx_empty : 1;
    // Set when a parity or framing error, or break interrupt is in the fifo buffer
    uint8_t error_in_fifo : 1;
} serial_reg_line_status;

#endif
