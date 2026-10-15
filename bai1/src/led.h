#ifndef LED_H
#define LED_H

#include "stm32f103xb.h"
#include <stdint.h>

#define LED_COUNT      3
#define LED_MAX_LEVEL  10   /* 0..10, mỗi bậc = 10% */

void pwm_init(void);                          /* TIM2 PWM 1kHz trên PA0, PA1, PA2 */
void led_set_level(uint8_t id, uint8_t level);/* id 0..2, level 0..10 */

#endif
