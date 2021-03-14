
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_reading;

void accel_init(void);

accel_reading accel_read(void);