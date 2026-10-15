# Bài tập 02: EXTI + Queue + UART1 trên STM32F103 (FreeRTOS)

- Nút PB12, ngắt ngoài EXTI cạnh xuống
- ISR tăng biến đếm, gửi vào Queue bằng xQueueSendFromISR, portYIELD_FROM_ISR
- Task UART chờ Blocked trên Queue, định dạng chuỗi, gửi qua USART1 (PA9/PA10, 115200)

## Build và nạp
    make
    make flash

## Video demo
<dán link YouTube>
