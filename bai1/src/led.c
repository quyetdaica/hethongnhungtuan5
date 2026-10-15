#include "led.h"

static const uint16_t duty_table[LED_MAX_LEVEL + 1] =
    {0, 10, 25, 50, 80, 120, 180, 260, 400, 650, 1000};

void pwm_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    GPIOA->CRL &= ~0xFFFu;
    GPIOA->CRL |=  0xAAAu;

    TIM2->PSC  = 7;
    TIM2->ARR  = 999;
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
    TIM2->CCR3 = 0;

    TIM2->CCMR1 = (6u << 4) | (1u << 3) | (6u << 12) | (1u << 11);
    TIM2->CCMR2 = (6u << 4) | (1u << 3);
    TIM2->CCER  = (1u << 0) | (1u << 4) | (1u << 8);

    TIM2->EGR = 1;
    TIM2->CR1 = (1u << 7) | 1u;
}

void led_set_level(uint8_t id, uint8_t level)
{
    if (level > LED_MAX_LEVEL) level = LED_MAX_LEVEL;
    uint32_t duty = duty_table[level];

    switch (id) {
    case 0: TIM2->CCR1 = duty; break;
    case 1: TIM2->CCR2 = duty; break;
    case 2: TIM2->CCR3 = duty; break;
    default: break;
    }
}
