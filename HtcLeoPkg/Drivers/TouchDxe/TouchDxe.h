#define TOUCH_TYPE_UNKNOWN  0
#define TOUCH_TYPE_B8       1
#define TOUCH_TYPE_68       2
#define TOUCH_TYPE_2A       3

int ts_type;

#define TYPE_2A_DEVID	(0x2A >> 1)
#define TYPE_B8_DEVID	(0xB8 >> 1)
#define TYPE_68_DEVID	(0x68 >> 1)

#define MAKEWORD(a, b)  ((uint16_t)(((uint8_t)(a)) | ((uint16_t)((uint8_t)(b))) << 8))

//static wait_queue_t ts_work __UNUSED;
//static struct timer poll_timer;