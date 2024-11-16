################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/LittleFS/src/fs_adstractions.c \
../Middlewares/Third_Party/LittleFS/src/lfs.c \
../Middlewares/Third_Party/LittleFS/src/lfs_util.c 

C_DEPS += \
./Middlewares/Third_Party/LittleFS/src/fs_adstractions.d \
./Middlewares/Third_Party/LittleFS/src/lfs.d \
./Middlewares/Third_Party/LittleFS/src/lfs_util.d 

OBJS += \
./Middlewares/Third_Party/LittleFS/src/fs_adstractions.o \
./Middlewares/Third_Party/LittleFS/src/lfs.o \
./Middlewares/Third_Party/LittleFS/src/lfs_util.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/LittleFS/src/%.o Middlewares/Third_Party/LittleFS/src/%.su Middlewares/Third_Party/LittleFS/src/%.cyclo: ../Middlewares/Third_Party/LittleFS/src/%.c Middlewares/Third_Party/LittleFS/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu18 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Core/app/sntp_app/inc -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-LittleFS-2f-src

clean-Middlewares-2f-Third_Party-2f-LittleFS-2f-src:
	-$(RM) ./Middlewares/Third_Party/LittleFS/src/fs_adstractions.cyclo ./Middlewares/Third_Party/LittleFS/src/fs_adstractions.d ./Middlewares/Third_Party/LittleFS/src/fs_adstractions.o ./Middlewares/Third_Party/LittleFS/src/fs_adstractions.su ./Middlewares/Third_Party/LittleFS/src/lfs.cyclo ./Middlewares/Third_Party/LittleFS/src/lfs.d ./Middlewares/Third_Party/LittleFS/src/lfs.o ./Middlewares/Third_Party/LittleFS/src/lfs.su ./Middlewares/Third_Party/LittleFS/src/lfs_util.cyclo ./Middlewares/Third_Party/LittleFS/src/lfs_util.d ./Middlewares/Third_Party/LittleFS/src/lfs_util.o ./Middlewares/Third_Party/LittleFS/src/lfs_util.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-LittleFS-2f-src

