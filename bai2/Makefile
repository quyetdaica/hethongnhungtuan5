.RECIPEPREFIX = >

TARGET  = firmware
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

RTOS = FreeRTOS-Kernel
DEV  = cmsis-device-f1

SRCS = src/main.c \
       $(DEV)/Source/Templates/system_stm32f1xx.c \
       $(RTOS)/tasks.c $(RTOS)/list.c $(RTOS)/queue.c \
       $(RTOS)/portable/GCC/ARM_CM3/port.c \
       $(RTOS)/portable/MemMang/heap_4.c
ASMS = $(DEV)/Source/Templates/gcc/startup_stm32f103xb.s

INCS = -Isrc -I$(RTOS)/include -I$(RTOS)/portable/GCC/ARM_CM3 \
       -I$(DEV)/Include -Icmsis_core/CMSIS/Core/Include

CFLAGS  = -mcpu=cortex-m3 -mthumb -Os -g -Wall -ffunction-sections -fdata-sections \
          -DSTM32F103xB $(INCS)
LDFLAGS = -mcpu=cortex-m3 -mthumb -Tlinker.ld -Wl,--gc-sections \
          --specs=nano.specs --specs=nosys.specs

OBJS = $(SRCS:.c=.o) $(ASMS:.s=.o)

all: $(TARGET).bin
> $(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
> $(CC) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
> $(OBJCOPY) -O binary $< $@

%.o: %.c
> $(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
> $(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET).bin
> st-flash write $(TARGET).bin 0x08000000

clean:
> rm -f $(OBJS) $(TARGET).elf $(TARGET).bin
