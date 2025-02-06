#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "bsp/board.h"
#include "tusb.h"

#define FIRST_GPIO 0

uint32_t kbcur = 0;
uint32_t kbprev = 0;
uint8_t midi_offset = 36;

uint8_t readrow(int row) {
	uint8_t res = 0;

	gpio_put(FIRST_GPIO + 0, row == 0? 0: 1);
	gpio_put(FIRST_GPIO + 1, row == 1? 0: 1);
	gpio_put(FIRST_GPIO + 10, row == 2? 0: 1);
	gpio_put(FIRST_GPIO + 11, row == 3? 0: 1);
	sleep_us(100);

	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 9)) << 0);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 8)) << 1);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 7)) << 2);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 6)) << 3);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 5)) << 4);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 4)) << 5);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 3)) << 6);
	res |= ((uint8_t)(gpio_get(FIRST_GPIO + 2)) << 7);

	return res;
}

void midi_task(void);

int main() {
	board_init();

	tusb_init();

	int gpio = FIRST_GPIO;
	for(int i = 0; i < 12; ++i, ++gpio) {
		// pins 0, 1, 10, 11 are output pins, the others are input
		int direction = (i < 2 || i > 9)? GPIO_OUT: GPIO_IN;
		gpio_init(gpio);
		gpio_set_dir(gpio, direction);
		// gpio_set_outover(gpio, GPIO_OVERRIDE_INVERT);
		if(direction == GPIO_IN) {
			// very important: use internal pullup resistors
			gpio_pull_up(gpio);
		}
	}

	while(1) {
		tud_task(); // tinyusb device task
		midi_task();
	}
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
	(void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

// Basic MIDI Messages
#define NOTE_OFF 0x80
#define NOTE_ON 0x90

void midi_task(void) {
	uint8_t static const key_map[32] = {
		59, 57, 55, 53, 51, 49, 61, 62,
		60, 58, 56, 54, 52, 50, 48, 63,
		36, 38, 40, 42, 44, 46, 64, 65,
		37, 39, 41, 43, 45, 47, 66, 67
	};

	kbprev = kbcur;
	kbcur = (uint32_t)(readrow(0));
	kbcur |= (uint32_t)(readrow(1)) << 8;
	kbcur |= (uint32_t)(readrow(2)) << 16;
	kbcur |= (uint32_t)(readrow(3)) << 24;

	uint32_t difference = kbcur ^ kbprev;
	if(!difference) return;

	for(int i = 0; i < 32; ++i) {
		uint32_t testbit = 1 << i;
		if(difference & testbit) {
			uint8_t note_val = midi_offset + key_map[i] - 36;
			if(kbcur & testbit) {
				// onte off
				uint8_t note_off[3] = {NOTE_OFF, note_val, 0};
				tud_midi_n_stream_write(0, 0, note_off, 3);
//				BLEMidiServer.noteOff(0, note_val, 127);
			} else {
				// note on
				uint8_t note_on[3] = {NOTE_ON, note_val, 127};
				tud_midi_n_stream_write(0, 0, note_on, 3);
//				BLEMidiServer.noteOn(0, note_val, 127);
			}
		}
	}
}


