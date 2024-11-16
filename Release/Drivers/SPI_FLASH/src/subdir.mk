################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SPI_FLASH/src/SPI_flash.c 

C_DEPS += \
./Drivers/SPI_FLASH/src/SPI_flash.d 

OBJS += \
./Drivers/SPI_FLASH/src/SPI_flash.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SPI_FLASH/src/%.o Drivers/SPI_FLASH/src/%.su Drivers/SPI_FLASH/src/%.cyclo: ../Drivers/SPI_FLASH/src/%.c Drivers/SPI_FLASH/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu18 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Core/app/sntp_app/inc -I../Core/app/ssdp_auto_discovery_app/inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SPI_FLASH-2f-src

clean-Drivers-2f-SPI_FLASH-2f-src:
	-$(RM) ./Drivers/SPI_FLASH/src/SPI_flash.cyclo ./Drivers/SPI_FLASH/src/SPI_flash.d ./Drivers/SPI_FLASH/src/SPI_flash.o ./Drivers/SPI_FLASH/src/SPI_flash.su

.PHONY: clean-Drivers-2f-SPI_FLASH-2f-src

