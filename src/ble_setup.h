

#define APP_TIMER_PRESCALER             0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         10                                          /**< Size of timer operation queues. */

uint16_t get_connection_handle();


void mouse_movement_send(int16_t x_delta, int16_t y_delta);

void ble_stack_init(void);

void peer_manager_init(void);

void advertising_init(void);

void gap_params_init(void);

void dis_init(void);

void bas_init(void);

void hids_init(void);

void services_init(void);

void conn_params_init(void);

void advertising_start(void);