#Buttons and Switches 
This application is used for interacting with different buttons and switches with in a application.
The status from the buttons/switches can always be read out and/or you can define a call back handler to respone to a button press or if the position of the switch is changed.
It support an total maximum of 8 buttons and switches. The application is fully interupt driven. 

The callback is called when a button is pressed or when a switch has moved his position. For the buttons also a long press events will be raised. The debounce time and the long pressed time can be changed in the project configuration. The default bounce time is 2mS and the default longpress time is 2S.

Example settings
```
# Application Specific settings
CONFIG_KEY_LONGPRESSED_TIME=1500
CONFIG_KEY_BOUNCE_TIME=3
```

The keys are defined in `` static const struct gpio_pin key_pins[]`` in ``key.c``. This contains for every device the GPIO node label name for the pin, the pin number and if it is a button or switch. 

```
struct gpio_pin {
	const char * const port;
	const uint8_t pin_number;
	const uint8_t keytype;
};

static const struct gpio_pin key_pins[] = {
	// Development kit buttons definition (nRF9160DK)
	{ DT_GPIO_LABEL(DT_ALIAS(button1), gpios), 
			DT_GPIO_PIN(DT_ALIAS(button1), gpios),
			KEY_TYPE_BUTTON	},
	{ DT_GPIO_LABEL(DT_ALIAS(button2), gpios), 
			DT_GPIO_PIN(DT_ALIAS(button2), gpios),
			KEY_TYPE_BUTTON	},
	// Development kit switches definition (nRF9160DK)
	{ DT_GPIO_LABEL(DT_ALIAS(switch1), gpios), 
			DT_GPIO_PIN(DT_ALIAS(switch1), gpios),
			KEY_TYPE_SWITCH	},	
	{ DT_GPIO_LABEL(DT_ALIAS(switch2), gpios), 
			DT_GPIO_PIN(DT_ALIAS(switch2), gpios),
			KEY_TYPE_SWITCH	},
	// Arrow key definition
	{ DT_GPIO_LABEL(DT_NODELABEL(arrowup), gpios), 
			DT_GPIO_PIN(DT_NODELABEL(arrowup), gpios),
			KEY_TYPE_BUTTON	},
	{ DT_GPIO_LABEL(DT_NODELABEL(arrowdown), gpios), 
			DT_GPIO_PIN(DT_NODELABEL(arrowdown), gpios),
			KEY_TYPE_BUTTON	},	
	{ DT_GPIO_LABEL(DT_NODELABEL(arrowleft), gpios), 
			DT_GPIO_PIN(DT_NODELABEL(arrowleft), gpios),
			KEY_TYPE_BUTTON	},	
	{ DT_GPIO_LABEL(DT_NODELABEL(arrowright), gpios), 
			DT_GPIO_PIN(DT_NODELABEL(arrowright), gpios),
			KEY_TYPE_BUTTON	}
};

```

The buttons/switches status is stored in a atomic byte named key_state. The position in the definition array corresponds with the bit position in the key_state. They are named in the ``key.h`` file.

```
#define KEY_NONE			0U 
#define	KEY_BUTTON1			1U
#define	KEY_BUTTON2			2U
#define	KEY_SWITCH1			4U
#define	KEY_SWITCH2			8U
#define	KEY_ARROW_UP		16U
#define	KEY_ARROW_DOWN		32U
#define	KEY_ARROW_LEFT		64U
#define	KEY_ARROW_RIGHT		128U
```

The routine needs to be intialized with the reference to the call-back program for responding to buttons/switch changes. 

```
int keys_buttons_init(key_handler_t key_handler)
```

The key handler call back routine should be defined like this 
```
static void key_handler(uint32_t key_state, uint32_t has_changed, uint8_t key_type)
```
keystate:		Contains all button and switch states
has_changed:	The button or switch that raised the event
				buttons only raise the event when pressed
				switch raised the event for on and off position
key_type:		Was the event raised by a button or switch or a long pressed button