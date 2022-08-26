#include <zephyr.h>
#include <device.h>

#include "keys.h"

//////////////////////////////////////////////////////////////////////////////////
static void key_handler(uint32_t key_state, uint32_t has_changed, uint8_t key_type)
{
	// keystate:	Contains all button and switch states
	// has_changed:	The button or switch that raised the event
	//				buttons only raise the event when pressed
	//				switch raised the event for on and off position
	// key_type:	Was the event raised by a button or switch or a long pressed button
	switch (key_type) {
	// A buttons is pressed
	case KEY_TYPE_BUTTON:
		if (has_changed & KEY_BUTTON1) {
			printk("Button 1 pressed.\n");
		}
		if (has_changed & KEY_BUTTON2) {
			printk("Button 2 pressed.\n");
		}
		if (has_changed & KEY_ARROW_UP) {
			printk("Button arrow UP pressed.\n");
		}
		if (has_changed & KEY_ARROW_DOWN){
			printk("Button arrow DOWN pressed.\n");
		}
		if (has_changed & KEY_ARROW_LEFT){
			printk("Button arrow LEFT pressed.\n");
		}
		if (has_changed & KEY_ARROW_RIGHT){
			printk("Button arrow RIGHT pressed.\n");
		}
		if (has_changed & KEY_ARROW_CONFIRM){
			printk("Button arrow CONFIRM pressed.\n");
		}
		break;
	// A switch has moved position
	case KEY_TYPE_SWITCH:
		if (has_changed & KEY_SWITCH1){
			if (key_state & KEY_SWITCH1) {
				printk("Switch 1 moved to state: ON\n");
			} else {
				printk("Switch 1 moved to state: OFF\n");
			}
		}
		if (has_changed & KEY_SWITCH2){
			printk("Switch changed to status : %d.\n", key_state & KEY_SWITCH2);
			if (key_state & KEY_SWITCH2) {
				printk("Switch 2 moved to state: ON\n");
			} else {
				printk("Switch 2 moved to state: OFF\n");
			}
		}
	// A button has been pressed and hold
	case KEY_TYPE_LONGPRESSED:
		if (has_changed & KEY_BUTTON1) {
			printk("Button 1 LONG pressed.\n");
		}
		if (has_changed & KEY_BUTTON2) {
			printk("Button 2 LONG pressed.\n");
		}
	default:
		// unkown key_type
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////
void main(void) {

	bool keys_buttons_available = true;
	uint32_t key_state;

	if (keys_buttons_init(key_handler)) {
		printk("ERROR: Failed to init switches and buttons, continue without input..\n");
		keys_buttons_available = false;	
	}

	key_state = keys_get();
	if (key_state & KEY_SWITCH1) { 
		printk("- Switch 1 state: ON\n");
	} else {
		printk("- Switch 1 state: OFF\n");
	}
	if (key_state & KEY_SWITCH2) { 
		printk("- Switch 2 state: ON\n");
	} else {
		printk("- Switch 2 state: OFF\n");
	}

}