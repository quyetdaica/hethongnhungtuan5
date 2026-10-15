#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>

#define BTN_SELECT_PIN  12
#define BTN_UP_PIN      13
#define BTN_DOWN_PIN    14

typedef enum { CMD_SELECT, CMD_UP, CMD_DOWN } LedCmd_t;

typedef struct {
    uint8_t  target;
    LedCmd_t cmd;
} LedMsg_t;

static QueueHandle_t led_queue[LED_COUNT];

static void buttons_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRH &= ~(0xFFFu << 16);
    GPIOB->CRH |=  (0x888u << 16);
    GPIOB->ODR |=  (1u << BTN_SELECT_PIN) | (1u << BTN_UP_PIN) | (1u << BTN_DOWN_PIN);
}

static uint8_t btn_pressed(uint8_t pin)
{
    return (GPIOB->IDR & (1u << pin)) ? 0 : 1;
}

static void delay_busy(volatile uint32_t n) { while (n--) {} }

/* Tu kiem tra luc khoi dong: LED1, LED2, LED3 lan luot sang het co */
static void startup_test(void)
{
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        led_set_level(i, LED_MAX_LEVEL);
        delay_busy(600000);
        led_set_level(i, 0);
    }
}

static void button_task(void *arg)
{
    (void)arg;
    uint8_t selected = 0;
    uint8_t prev[3] = {0, 0, 0};

    for (;;) {
        uint8_t now[3];
        now[0] = btn_pressed(BTN_SELECT_PIN);
        now[1] = btn_pressed(BTN_UP_PIN);
        now[2] = btn_pressed(BTN_DOWN_PIN);

        LedMsg_t msg;
        int send = 0;
        msg.target = selected;
        msg.cmd = CMD_UP;

        if (now[0] && !prev[0]) {
            selected = (uint8_t)((selected + 1) % LED_COUNT);
            msg.target = selected;
            msg.cmd = CMD_SELECT;
            send = 1;
        } else if (now[1] && !prev[1]) {
            msg.cmd = CMD_UP;
            send = 1;
        } else if (now[2] && !prev[2]) {
            msg.cmd = CMD_DOWN;
            send = 1;
        }

        if (send) {
            for (int i = 0; i < LED_COUNT; i++) {
                xQueueSend(led_queue[i], &msg, 0);
            }
        }

        prev[0] = now[0];
        prev[1] = now[1];
        prev[2] = now[2];

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void led_task(void *param)
{
    uint8_t id = (uint8_t)(uintptr_t)param;
    uint8_t level = 3;
    LedMsg_t msg;

    led_set_level(id, level);

    for (;;) {
        if (xQueueReceive(led_queue[id], &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.target != id) continue;

            switch (msg.cmd) {
            case CMD_UP:
                if (level < LED_MAX_LEVEL) level++;
                led_set_level(id, level);
                break;
            case CMD_DOWN:
                if (level > 0) level--;
                led_set_level(id, level);
                break;
            case CMD_SELECT:
                led_set_level(id, LED_MAX_LEVEL);
                vTaskDelay(pdMS_TO_TICKS(200));
                led_set_level(id, level);
                break;
            }
        }
    }
}

int main(void)
{
    pwm_init();
    buttons_init();
    startup_test();

    for (int i = 0; i < LED_COUNT; i++) {
        led_queue[i] = xQueueCreate(4, sizeof(LedMsg_t));
    }

    xTaskCreate(led_task, "LED1", 128, (void *)(uintptr_t)0, 1, NULL);
    xTaskCreate(led_task, "LED2", 128, (void *)(uintptr_t)1, 1, NULL);
    xTaskCreate(led_task, "LED3", 128, (void *)(uintptr_t)2, 1, NULL);
    xTaskCreate(button_task, "BTN", 128, NULL, 2, NULL);

    vTaskStartScheduler();
    for (;;) {}
}

void vApplicationMallocFailedHook(void)
{
    for (;;) {}
}
