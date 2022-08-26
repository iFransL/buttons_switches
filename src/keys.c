#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include "keys.h"

//////////////////////////////////////////////////////////////////////////////////
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

static const struct device *key_devs[ARRAY_SIZE(key_pins)];
static struct gpio_callback dev_gpio_button_cb;
static struct k_spinlock lock;
static struct k_work_delayable buttons_scan; 
static struct k_work_delayable long_pressed;
static atomic_t my_keys;
static key_handler_t key_handler_cb;
static bool long_pressed_state; 

K_SEM_DEFINE(wait_initial_run, 0, 1);

//////////////////////////////////////////////////////////////////////////////////
static int ctrl_pin_interrupt(bool state)
{
	int err = 0;

	for (size_t i = 0; i < ARRAY_SIZE(key_pins) && !err; i++) {
		if (state) {
			err = gpio_pin_interrupt_configure(key_devs[i], 
					key_pins[i].pin_number, GPIO_INT_EDGE_BOTH);
		} else {
			err = gpio_pin_interrupt_configure(key_devs[i], 
					key_pins[i].pin_number, GPIO_INT_DISABLE);
		}
	}
	return err;
}

//////////////////////////////////////////////////////////////////////////////////
static void long_pressed_fn(struct k_work *k_work) 
{
    // printk("long press ended...\n");
    long_pressed_state = false;
    if (key_handler_cb != NULL) {
        uint32_t keys_state = atomic_get(&my_keys);
        key_handler_cb(keys_state, keys_state, KEY_TYPE_LONGPRESSED);
    }
}

//////////////////////////////////////////////////////////////////////////////////
static void buttons_scan_fn(struct k_work *k_work)
{
	int err = 0;
	uint32_t keys_state = 0, switch_changed = 0;
	bool button_pressed = false, switch_moved = false;
	static bool initial_run = true; 
	
	static uint32_t last_keys_state, last_switch_state;

	for (size_t i = 0; i < ARRAY_SIZE(key_pins); i++) {
		int val;
		val = gpio_pin_get_raw(key_devs[i], key_pins[i].pin_number);
		if (val<0) { 
			printk("Cannot read GPIO\n");	
		} 
		if (!val) { 
			keys_state |= 1U << i; 
			if (!(last_keys_state & 1U<<i)) {
				switch (key_pins[i].keytype) {
				case KEY_TYPE_BUTTON:
					button_pressed = true;
					break;
				case KEY_TYPE_SWITCH:
					switch_moved = true; 
					switch_changed |= 1U << i; 
					break;
				default: 
					break;
				}
			}
		} 
		else {
			if (last_keys_state & 1U<<i) {						
				if (key_pins[i].keytype == KEY_TYPE_SWITCH )  { 
					switch_moved = true; 
					switch_changed |= 1U << i; 						
				} else { 
					// Cancel long press 
                    if (long_pressed_state) {
                        k_work_cancel_delayable(&long_pressed);
                    }
				}
			} 
		}
	}
	atomic_set(&my_keys, (atomic_val_t)keys_state);
	err = ctrl_pin_interrupt(true);

	if (!initial_run){
		uint32_t has_changed = (keys_state ^ last_keys_state);
		if (button_pressed) { 
			// Start long press 
            long_pressed_state = true;
            k_work_schedule(&long_pressed, K_MSEC(KEY_LONGPRESSED_TIME));
		}
        if ((key_handler_cb != NULL) & (button_pressed)) {
            key_handler_cb(keys_state, has_changed, KEY_TYPE_BUTTON);
        }
        if ((key_handler_cb != NULL) & (switch_moved)) {
            key_handler_cb(keys_state, has_changed, KEY_TYPE_SWITCH);
        }

	} else {
		initial_run = false;
		k_sem_give(&wait_initial_run);
	}

	last_keys_state = keys_state;
    last_switch_state = switch_changed;
}

//////////////////////////////////////////////////////////////////////////////////
void button_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int err = 0;
	k_spinlock_key_t key = k_spin_lock(&lock);
        err = ctrl_pin_interrupt(false);
        k_work_reschedule(&buttons_scan, K_MSEC(KEY_BOUNCE_TIME));
	k_spin_unlock(&lock, key);
}

//////////////////////////////////////////////////////////////////////////////////
int keys_buttons_init(key_handler_t key_handler)
{
	int err;
	uint32_t pin_mask = 0;
    key_handler_cb = key_handler;
    long_pressed_state = false;

	for (size_t i = 0; i < ARRAY_SIZE(key_pins); i++) {
		key_devs[i] = device_get_binding(key_pins[i].port);
		if (!key_devs[i]) {
			printk("Cannot bind gpio device\n");
			return -1;
		}
		err = gpio_pin_configure(key_devs[i], key_pins[i].pin_number, GPIO_INPUT | GPIO_PULL_UP);
		if (err) {
			printk("Cannot configure button gpio\n");
			return -1;
		}
		err = gpio_pin_interrupt_configure(key_devs[i], key_pins[i].pin_number, GPIO_INT_EDGE_BOTH);
		if (err) {
			printk("Failed to configure interrupt for button gpio, %d, \n",i);
			return -1;
		}
		pin_mask |= BIT(key_pins[i].pin_number);
	}
	gpio_init_callback(&dev_gpio_button_cb, button_interrupt, pin_mask);

	for (size_t i = 0; i < ARRAY_SIZE(key_pins); i++) {
		err = gpio_add_callback(key_devs[i], &dev_gpio_button_cb);
		if (err) {
			printk("Failed to interrupt call back to button gpio\n");
			return -1;
		}
	}
	k_work_init_delayable(&buttons_scan, buttons_scan_fn);
    k_work_init_delayable(&long_pressed, long_pressed_fn);

	k_work_schedule(&buttons_scan, K_NO_WAIT);
	k_sem_take(&wait_initial_run, K_FOREVER);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
uint32_t keys_get(void)
{
	return atomic_get(&my_keys);
}