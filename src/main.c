#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "bsp.h"
#include "app_scheduler.h"
#include "softdevice_handler_appsh.h"
#include "app_timer_appsh.h"
#include "ble_advertising.h"
#include "fds.h"
#include "fstorage.h"
#include "ble_setup.h"
#include "accel.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

APP_TIMER_DEF(read_accel_timer);

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, \
                                            BLE_STACK_HANDLER_SCHED_EVT_SIZE)                     /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                10                                                        /**< Maximum number of events in the scheduler queue. */

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


static int16_t map_speeds(int16_t in) {
    if(in < 0) {
        return -map_speeds(-in);
    }

    if (in > 1500)
    {
        return 15;
    }
    else if (in > 1200) {
        return 10;
    }
    else if (in > 800) {
        return 5;
    }
    else if (in > 600)
    {
        return 3;
    }
    else if (in > 400)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void read_accel_timer_handler(void * p_context)
{
    accel_reading acc = accel_read();
    int16_t mouse_x = 0, mouse_y = 0;

    mouse_x = map_speeds(acc.x);
    mouse_y = map_speeds(acc.y);
    //NRF_LOG_INFO("Got %d, %d\r\n", mouse_x, mouse_y);

    if(get_connection_handle() != BLE_CONN_HANDLE_INVALID) {
        mouse_movement_send(mouse_y, -mouse_x);
    }
}

int main(void)
{
    uint32_t err_code;

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    APP_ERROR_CHECK(err_code);

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

    ble_stack_init();
    peer_manager_init();
    gap_params_init();
    advertising_init();
    services_init();
    conn_params_init();
    advertising_start();

    err_code = app_timer_create(&read_accel_timer,
                                APP_TIMER_MODE_REPEATED,
                                read_accel_timer_handler);

    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(read_accel_timer, APP_TIMER_TICKS(50, APP_TIMER_PRESCALER), NULL);

    APP_ERROR_CHECK(err_code);

    accel_init();

    NRF_LOG_INFO("Application started!\r\n");

    for (;;)
    {
        app_sched_execute();
        if (NRF_LOG_PROCESS() == false)
        {
            err_code = sd_app_evt_wait();
            APP_ERROR_CHECK(err_code);
        }
    }
}

