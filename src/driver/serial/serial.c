#include "driver/serial/serial.h"
#include "utility/bitcast.h"
#include "core/io.h"

void serial_init(uint16_t port, struct serial_init_info init_info) {
    struct serial_reg_enable_int int_info = {
        .enable_data_available_int = 0,
        .enable_tx_holding_empty_int = 0,
        .enable_line_status_int = 0,
        .enable_modem_status_int = 0,
    };
    io_out8(port + SERIAL_REG_ENABLE_INT, BITCAST(uint8_t, int_info));

    uint16_t line_control_port = port + SERIAL_REG_LINE_CONTROL;
    struct serial_reg_line_control line_control = {
        .data_size = init_info.data_size,
        .stop_bits = init_info.stop_bits,
        .parity = init_info.parity,
        .enable_break = init_info.enable_break,
        .divisor_latch_access = true
    };
    io_out8(line_control_port, BITCAST(uint8_t, line_control));

    io_out8(port + SERIAL_REG_DIVISOR_LOW, init_info.baudrate_divisor & 0xFF);
    io_out8(port + SERIAL_REG_DIVISOR_HIGH, (init_info.baudrate_divisor >> 8) & 0xFF);

    line_control.divisor_latch_access = false;
    io_out8(line_control_port, BITCAST(uint8_t, line_control));

    io_out8(port + SERIAL_REG_MODEM_CONTROL, 0);

    struct serial_reg_fifo_control fifo_control = {
        .fifo_enable = true,
        .clear_rx_buffer = true,
        .clear_tx_buffer = true,
        .trigger_level = SERIAL_TRIGGER_LEVEL_14_BYTES
    };
    io_out8(port + SERIAL_REG_FIFO_CONTROL, BITCAST(uint8_t, fifo_control));
}

void serial_set_baudrate_divisor(uint16_t port, uint16_t divisor) {
    // TODO: Disable interrupts when accessing divisor?
    uint16_t line_control_port = port + SERIAL_REG_LINE_CONTROL;
    struct serial_reg_line_control line_control = BITCAST(struct serial_reg_line_control, io_in8(line_control_port));
    line_control.divisor_latch_access = true;
    io_out8(line_control_port, BITCAST(uint8_t, line_control));

    io_out8(port + SERIAL_REG_DIVISOR_LOW, divisor & 0xFF);
    io_out8(port + SERIAL_REG_DIVISOR_HIGH, (divisor >> 8) & 0xFF);

    line_control.divisor_latch_access = false;
    io_out8(line_control_port, BITCAST(uint8_t, line_control));
}

bool serial_data_available(uint16_t port) {
    uint16_t status_port = port + SERIAL_REG_LINE_STATUS;
    struct serial_reg_line_status status = BITCAST(struct serial_reg_line_status, io_in8(status_port));
    return status.data_available;
}

bool serial_tx_ready(uint16_t port) {
    uint16_t status_port = port + SERIAL_REG_LINE_STATUS;
    struct serial_reg_line_status status = BITCAST(struct serial_reg_line_status, io_in8(status_port));
    return status.tx_holding_empty;
}

uint8_t serial_busy_read(uint16_t port) {
    while (!serial_data_available(port))
        continue;

    return io_in8(port);
}

void serial_busy_write(uint16_t port, uint8_t data) {
    while (!serial_tx_ready(port))
        continue;

    io_out8(port, data);
}
