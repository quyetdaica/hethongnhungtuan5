#include "stm32f103xb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <stdio.h>

#define BTN_PIN      12          /* PB12 -> EXTI line 12 */
#define DEBOUNCE_MS  200

static QueueHandle_t count_queue;
static volatile uint32_t press_count = 0;   /* bien dem so lan nhan */

/* ---------- UART1: PA9 TX, PA10 RX, 115200 8N1 ---------- */
static void uart1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    /* PA9: AF push-pull 50MHz (0xB), PA10: input floating (0x4) */
    GPIOA->CRH &= ~(0xFFu << 4);
    GPIOA->CRH |=  (0xBu << 4) | (0x4u << 8);

    USART1->BRR = 69;                        /* 8MHz / 115200 */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void uart1_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = (uint8_t)c;
}

static void uart1_puts(const char *s)
{
    while (*s) uart1_putc(*s++);
    while (!(USART1->SR & USART_SR_TC)) {}
}

/* ---------- Ngat ngoai EXTI: PB12, canh xuong ---------- */
static void exti_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    /* PB12: input pull-up (CNF=10, MODE=00, ODR=1) */
    GPIOB->CRH &= ~(0xFu << 16);
    GPIOB->CRH |=  (0x8u << 16);
    GPIOB->BSRR = (1u << BTN_PIN);

    /* EXTI line 12 -> port B (EXTICR[3], bit 3:0) */
    AFIO->EXTICR[3] &= ~(0xFu << 0);
    AFIO->EXTICR[3] |=  (0x1u << 0);

    EXTI->IMR  |= (1u << BTN_PIN);
    EXTI->FTSR |= (1u << BTN_PIN);           /* canh xuong */
    EXTI->RTSR &= ~(1u << BTN_PIN);

    /* Uu tien phai >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) */
    NVIC_SetPriority(EXTI15_10_IRQn, 6);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void EXTI15_10_IRQHandler(void)
{
    static TickType_t last_tick = 0;

    if (EXTI->PR & (1u << BTN_PIN)) {
        EXTI->PR = (1u << BTN_PIN);          /* xoa co ngat */

        TickType_t now = xTaskGetTickCountFromISR();
        if ((now - last_tick) >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
            last_tick = now;

            press_count++;                   /* 1. tang bien dem */

            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            uint32_t value = press_count;
            xQueueSendFromISR(count_queue, &value,       /* 2. gui vao Queue */
                              &xHigherPriorityTaskWoken);

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);/* 3. chuyen ngu canh */
        }
    }
}

/* ---------- Task UART ---------- */
static void uart_task(void *arg)
{
    (void)arg;
    uint32_t value;
    char buf[48];

    uart1_puts("\r\nSTM32F103 FreeRTOS - EXTI + Queue + UART1 san sang\r\n");

    for (;;) {
        /* Blocked cho den khi Queue co du lieu */
        if (xQueueReceive(count_queue, &value, portMAX_DELAY) == pdTRUE) {
            snprintf(buf, sizeof(buf), "So lan nhan nut: %lu\r\n",
                     (unsigned long)value);
            uart1_puts(buf);
        }
    }
}

int main(void)
{
    uart1_init();

    count_queue = xQueueCreate(8, sizeof(uint32_t));
    xTaskCreate(uart_task, "UART", 256, NULL, 1, NULL);

    exti_init();                             /* bat ngat sau khi tao Queue */

    vTaskStartScheduler();
    for (;;) {}
}

void vApplicationMallocFailedHook(void)
{
    for (;;) {}
}
