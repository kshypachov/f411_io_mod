################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/app/sntp_app/src/sntp.c 

C_DEPS += \
./Core/app/sntp_app/src/sntp.d 

OBJS += \
./Core/app/sntp_app/src/sntp.o 


# Each subdirectory must supply rules for building sources it contributes
Core/app/sntp_app/src/%.o Core/app/sntp_app/src/%.su Core/app/sntp_app/src/%.cyclo: ../Core/app/sntp_app/src/%.c Core/app/sntp_app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Core/app/sntp_app/inc -I../Core/app/ssdp_auto_discovery_app/inc -I../Core/app/ModBusTCP_server/inc -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/include -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/port -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/tcp -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-app-2f-sntp_app-2f-src

clean-Core-2f-app-2f-sntp_app-2f-src:
	-$(RM) ./Core/app/sntp_app/src/sntp.cyclo ./Core/app/sntp_app/src/sntp.d ./Core/app/sntp_app/src/sntp.o ./Core/app/sntp_app/src/sntp.su

.PHONY: clean-Core-2f-app-2f-sntp_app-2f-src

