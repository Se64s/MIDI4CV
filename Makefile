######################################
# user tools
######################################
# Clean tool
#RM = -rm -fR
RM = del /q


######################################
# target
######################################
TARGET = MIDI4CV


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# User definitions
#######################################
# Application tag number
APP_TAG = "0.0.1"

# Application name
APP_NAME = "MIDI4CV"


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
App/Src/main.c \
App/Src/cli_task.c \
App/Src/cli_cmd.c \
App/Src/midi_task.c \
App/Src/cv_ctrl_task.c \
BSP/Src/stm32g0xx_it.c \
BSP/Src/stm32g0xx_hal_msp.c \
BSP/Src/system_stm32g0xx.c \
BSP/Src/sys_mcu.c \
BSP/Src/sys_rtos.c \
BSP/Src/sys_serial.c \
BSP/Src/sys_ll_serial.c \
BSP/Src/sys_gpio.c \
BSP/Src/sys_i2c.c \
Lib/cbuf/circular_buffer.c \
Lib/printf/printf.c \
Lib/midi/midi_lib.c \
Lib/dac_mcp4728/dac_mcp4728.c \
Lib/UserError/user_error.c \
Lib/CrashCatcher/Core/src/CrashCatcher.c \
Lib/CrashCatcher/Usr/src/crash_hexdump.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_gpio.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_ll_gpio.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_ll_rcc.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_ll_dma.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_cortex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_exti.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_i2c.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_i2c_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_ll_usart.c \
RTOS/FreeRTOS/Source/croutine.c \
RTOS/FreeRTOS/Source/event_groups.c \
RTOS/FreeRTOS/Source/list.c \
RTOS/FreeRTOS/Source/queue.c \
RTOS/FreeRTOS/Source/stream_buffer.c \
RTOS/FreeRTOS/Source/tasks.c \
RTOS/FreeRTOS/Source/timers.c \
RTOS/FreeRTOS/Source/portable/GCC/ARM_CM0/port.c \
RTOS/FreeRTOS/Source/portable/MemMang/heap_4.c \
RTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI/FreeRTOS_CLI.c \


# ASM sources
ASM_SOURCES = \
startup_stm32g070xx.s \
Lib/CrashCatcher/Core/src/CrashCatcher_armv6m.s \


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =  

# C defines
C_DEFS =  \
-DCUSTOM_HARD_FAULT \
-DUSE_HAL_DRIVER \
-DUSE_FULL_LL_DRIVER \
-DSTM32G070xx \
-DUSE_USER_ASSERT \
-DUSE_USER_RTOS \
-DUSE_USER_RTOS_TICK \


# AS includes
AS_INCLUDES = 


# C includes
C_INCLUDES =  \
-IApp/Inc \
-IBSP/Inc \
-ILib/cbuf \
-ILib/printf \
-ILib/UserError \
-ILib/midi \
-ILib/dac_mcp4728 \
-ILib/CrashCatcher/include \
-ILib/CrashCatcher/Core/src \
-IDrivers/STM32G0xx_HAL_Driver/Inc \
-IDrivers/STM32G0xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32G0xx/Include \
-IDrivers/CMSIS/Include \
-IRTOS/FreeRTOS/Source/include \
-IRTOS/FreeRTOS/Source/portable/GCC/ARM_CM0 \
-IRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI \


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# ADD USER DEFINES
#######################################

C_DEFS += -DMAIN_APP_VERSION=\"$(APP_TAG)\"
C_DEFS += -DMAIN_APP_NAME=\"$(APP_NAME)\"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32G070RBTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	$(RM) $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***