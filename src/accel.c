#include <stdio.h>
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "bsp.h"
#include "app_util_platform.h"
#include "accel.h"

#define NRF_LOG_MODULE_NAME "ACCEL"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* MSA301 Regs */
#define MSA301_ADDR 0x26
#define MSA301_PART_ID 0x13
#define REG_PART_ID 0x01
#define REG_DATA 0x02
#define REG_ODR 0x10
#define REG_PWR 0x11

/* Pins */
#define SDA_PIN 25
#define SCL_PIN 28
#define PWR_PIN 24



/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

// Convert buffer data to int16_t
#define CONV_DATA(lsb, msb) ((int16_t)(lsb + (msb << 8)) >> 2) 

accel_reading offset;

/**
 * @brief UART initialization.
 */
static void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = SCL_PIN,
       .sda                = SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void accel_twi_read(uint8_t addr, uint8_t* buffer, int len) {

    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, MSA301_ADDR, &addr, 1, true);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_rx(&m_twi, MSA301_ADDR, buffer, len);
    APP_ERROR_CHECK(err_code);

}

void accel_twi_write(uint8_t addr, uint8_t* buffer, int len) {
    uint8_t* _buffer = (uint8_t*) malloc(sizeof(uint8_t)*len);
    _buffer[0] = addr;
    memcpy(&_buffer[1], buffer, sizeof(uint8_t)*len);

    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, MSA301_ADDR, _buffer, len+1, false);
    APP_ERROR_CHECK(err_code);
    free(_buffer);
}

void accel_init(void) {

    // We power the accelerometer from a GPIO
    // This is fine as the current draw is very low
    nrf_gpio_cfg_output(PWR_PIN);
    nrf_gpio_pin_write(PWR_PIN, 1);

    twi_init();

    uint8_t buffer = 0x05;
    accel_twi_write(REG_ODR, &buffer, 1);

    buffer = 0x1e;
    accel_twi_write(REG_PWR, &buffer, 1);

    accel_twi_read(REG_PART_ID, &buffer, 1);
    if(buffer == MSA301_PART_ID) {
        NRF_LOG_INFO("MSA301 initialized\r\n");
    } else {
        NRF_LOG_ERROR("MSA301 wrong part ID: %02x, expected %02x\r\n", buffer, MSA301_PART_ID);
    }

    accel_twi_read(REG_ODR, &buffer, 1);

    offset = accel_read();

    NRF_LOG_ERROR("MSA301 odr: %02x\r\n", buffer);
}


accel_reading accel_read(void) {
    uint8_t buffer[6];
    accel_reading out;
    accel_twi_read(REG_DATA, buffer, 6);
    out.x = CONV_DATA(buffer[0], buffer[1]) - offset.x;
    out.y = CONV_DATA(buffer[2], buffer[3]) - offset.y;
    out.z = CONV_DATA(buffer[4], buffer[5]) - offset.z;

    return out;
}