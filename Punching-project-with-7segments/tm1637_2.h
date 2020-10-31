/**
 * Copyright (c) 2017-2018, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 *
 * This is ATtiny13/25/45/85 library for 4-Digit LED Display based on TM1637_2 chip.
 *
 * Features:
 * - display raw segments
 * - display digits
 * - display colon
 * - display on/off
 * - brightness control
 *
 * References:
 * - library: https://github.com/lpodkalicki/attiny-TM1637_2-library
 * - documentation: https://github.com/lpodkalicki/attiny-TM1637_2-library/README.md
 * - TM1637_2 datasheet: https://github.com/lpodkalicki/attiny-TM1637_2-library/blob/master/docs/TM1637_2_V2.4_EN.pdf
 */

#ifndef	_ATTINY_TM1637_2_H_
#define	_ATTINY_TM1637_2_H_

#include <stdint.h>

// Main Settings
#define	TM1637_2_DIO_PIN			PD6
#define	TM1637_2_CLK_PIN			PD5
#define	TM1637_2_DELAY_US			(5)
#define	TM1637_2_BRIGHTNESS_MAX		(7)
#define	TM1637_2_POSITION_MAX		(4)

// TM1637_2 commands
#define	TM1637_2_CMD_SET_DATA		0x40
#define	TM1637_2_CMD_SET_ADDR		0xC0
#define	TM1637_2_CMD_SET_DSIPLAY		0x80

// TM1637_2 data settings (use bitwise OR to contruct complete command)
#define	TM1637_2_SET_DATA_WRITE		0x00 // write data to the display register
#define	TM1637_2_SET_DATA_READ		0x02 // read the key scan data
#define	TM1637_2_SET_DATA_A_ADDR		0x00 // automatic address increment
#define	TM1637_2_SET_DATA_F_ADDR		0x04 // fixed address
#define	TM1637_2_SET_DATA_M_NORM		0x00 // normal mode
#define	TM1637_2_SET_DATA_M_TEST		0x10 // test mode

// TM1637_2 display control command set (use bitwise OR to consruct complete command)
#define	TM1637_2_SET_DISPLAY_OFF		0x00 // off
#define	TM1637_2_SET_DISPLAY_ON		0x08 // on


/**
 * Initialize TM1637_2 display driver.
 * Clock pin (TM1637_2_CLK_PIN) and data pin (TM1637_2_DIO_PIN)
 * are defined at the top of this file.
 */
void TM1637_2_init(const uint8_t enable, const uint8_t brightness);

/**
 * Turn display on/off.
 * value: 1 - on, 0 - off
 */
void TM1637_2_enable(const uint8_t value);

/**
 * Set display brightness.
 * Min value: 0
 * Max value: 7
 */
void TM1637_2_set_brightness(const uint8_t value);

/**
 * Display raw segments at position (0x00..0x03)
 *
 *      bits:
 *        -- 0 --
 *       |       |
 *       5       1
 *       |       |
 *        -- 6 --
 *       |       |
 *       4       2
 *       |       |
 *        -- 3 --
 *
 * Example segment configurations:
 * - for character 'H', segments=0b01110110
 * - for character '-', segments=0b01000000
 * - etc.
 */
void TM1637_2_display_segments(const uint8_t position, const uint8_t segments);

/**
 * Display digit ('0'..'9') at position (0x00..0x03)
 */
void TM1637_2_display_digit(const uint8_t position, const uint8_t digit);

/**
 * Display colon on/off.
 * value: 1 - on, 0 - off
 */
void TM1637_2_display_colon(const uint8_t value);

/**
 * Clear all segments (including colon).
 */
void TM1637_2_clear(void);

#endif	/* !_ATTINY_TM1637_2_H_ */
