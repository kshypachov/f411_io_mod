################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/validation/src/subdir.mk
-include Middlewares/Third_Party/mongoose/subdir.mk
-include Middlewares/Third_Party/LittleFS/src/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/i2c_display/src/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Drivers/SPI_FLASH/src/subdir.mk
-include Core/app/ssdp_auto_discovery_app/src/subdir.mk
-include Core/app/sntp_app/src/subdir.mk
-include Core/app/mqtt_client_app/src/subdir.mk
-include Core/app/http_server_app/src/subdir.mk
-include Core/app/ModBusTCP_server/src/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/tcp/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/rtu/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/port/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/ascii/subdir.mk
-include Core/app/ModBusTCP_server/freemodbus_1.6/modbus/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CCM_DEPS)),)
-include $(CCM_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CXXM_DEPS)),)
-include $(CXXM_DEPS)
endif
ifneq ($(strip $(C++M_DEPS)),)
-include $(C++M_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := f411_io_mod
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
f411_io_mod.elf \

MAP_FILES += \
f411_io_mod.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
f411_io_mod.list \

OBJCOPY_HEX += \
f411_io_mod.hex \

OBJCOPY_BIN += \
f411_io_mod.bin \


# All Target
all: main-build

# Main-build Target
main-build: f411_io_mod.elf secondary-outputs

# Tool invocations
f411_io_mod.elf f411_io_mod.map: $(OBJS) $(USER_OBJS) ../STM32F411CEUX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-g++ -o "f411_io_mod.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"../STM32F411CEUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="f411_io_mod.map" -Wl,--gc-sections -static --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

f411_io_mod.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "f411_io_mod.list"
	@echo 'Finished building: $@'
	@echo ' '

f411_io_mod.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "f411_io_mod.hex"
	@echo 'Finished building: $@'
	@echo ' '

f411_io_mod.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "f411_io_mod.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout f411_io_mod.bin f411_io_mod.elf f411_io_mod.hex f411_io_mod.list f411_io_mod.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
