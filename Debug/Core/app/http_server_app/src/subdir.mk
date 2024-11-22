################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/app/http_server_app/src/dashboard.c 

C_DEPS += \
./Core/app/http_server_app/src/dashboard.d 

OBJS += \
./Core/app/http_server_app/src/dashboard.o 


# Each subdirectory must supply rules for building sources it contributes
Core/app/http_server_app/src/%.o Core/app/http_server_app/src/%.su Core/app/http_server_app/src/%.cyclo: ../Core/app/http_server_app/src/%.c Core/app/http_server_app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/mbedtls -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/psa -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/library -I../Core/app/sntp_app/inc -I../Core/app/ssdp_auto_discovery_app/inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-app-2f-http_server_app-2f-src

clean-Core-2f-app-2f-http_server_app-2f-src:
	-$(RM) ./Core/app/http_server_app/src/dashboard.cyclo ./Core/app/http_server_app/src/dashboard.d ./Core/app/http_server_app/src/dashboard.o ./Core/app/http_server_app/src/dashboard.su

.PHONY: clean-Core-2f-app-2f-http_server_app-2f-src

