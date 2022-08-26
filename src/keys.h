#ifndef KEYS_H__
#define KEYS_H__

#ifdef CONFIG_KEY_BOUNCE_TIME
    #define KEY_BOUNCE_TIME	CONFIG_KEY_BOUNCE_TIME
#else
    #define KEY_BOUNCE_TIME	2
#endif

#ifdef CONFIG_KEY_LONGPRESSED_TIME
    #define KEY_LONGPRESSED_TIME CONFIG_KEY_LONGPRESSED_TIME
#else
    #define KEY_LONGPRESSED_TIME 2000
#endif

enum key_type { KEY_TYPE_BUTTON, KEY_TYPE_SWITCH, KEY_TYPE_LONGPRESSED, KEY_TYPE_LONGPR_RELEASED, KEY_TYPE_DOUBLE_CLICK };
#define KEY_NONE			0U 
#define	KEY_BUTTON1			1U
#define	KEY_BUTTON2			2U
#define	KEY_SWITCH1			4U
#define	KEY_SWITCH2			8U
#define	KEY_ARROW_UP		16U
#define	KEY_ARROW_DOWN		32U
#define	KEY_ARROW_LEFT		64U
#define	KEY_ARROW_RIGHT		128U
#define	KEY_ARROW_CONFIRM	1U		// Equal to button 1

typedef void (*key_handler_t)(uint32_t key_state, uint32_t has_changed, uint8_t key_type);

int keys_buttons_init(key_handler_t key_handler);
uint32_t keys_get(void);

#endif /* KEYS_H__ */