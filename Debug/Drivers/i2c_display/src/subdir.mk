################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/i2c_display/src/fonts.c \
../Drivers/i2c_display/src/ssd1306.c 

C_DEPS += \
./Drivers/i2c_display/src/fonts.d \
./Drivers/i2c_display/src/ssd1306.d 

OBJS += \
./Drivers/i2c_display/src/fonts.o \
./Drivers/i2c_display/src/ssd1306.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/i2c_display/src/%.o Drivers/i2c_display/src/%.su Drivers/i2c_display/src/%.cyclo: ../Drivers/i2c_display/src/%.c Drivers/i2c_display/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/mbedtls -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/psa -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/library -I../Core/app/sntp_app/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-i2c_display-2f-src

clean-Drivers-2f-i2c_display-2f-src:
	-$(RM) ./Drivers/i2c_display/src/fonts.cyclo ./Drivers/i2c_display/src/fonts.d ./Drivers/i2c_display/src/fonts.o ./Drivers/i2c_display/src/fonts.su ./Drivers/i2c_display/src/ssd1306.cyclo ./Drivers/i2c_display/src/ssd1306.d ./Drivers/i2c_display/src/ssd1306.o ./Drivers/i2c_display/src/ssd1306.su

.PHONY: clean-Drivers-2f-i2c_display-2f-src

