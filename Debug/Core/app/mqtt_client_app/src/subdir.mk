################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/app/mqtt_client_app/src/home_mqtt_gen.cpp \
../Core/app/mqtt_client_app/src/mqtt.cpp \
../Core/app/mqtt_client_app/src/mqtt_gen_strings.cpp 

OBJS += \
./Core/app/mqtt_client_app/src/home_mqtt_gen.o \
./Core/app/mqtt_client_app/src/mqtt.o \
./Core/app/mqtt_client_app/src/mqtt_gen_strings.o 

CPP_DEPS += \
./Core/app/mqtt_client_app/src/home_mqtt_gen.d \
./Core/app/mqtt_client_app/src/mqtt.d \
./Core/app/mqtt_client_app/src/mqtt_gen_strings.d 


# Each subdirectory must supply rules for building sources it contributes
Core/app/mqtt_client_app/src/%.o Core/app/mqtt_client_app/src/%.su Core/app/mqtt_client_app/src/%.cyclo: ../Core/app/mqtt_client_app/src/%.cpp Core/app/mqtt_client_app/src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Core/app/sntp_app/inc -I../Core/app/ModBusTCP_server/inc -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/include -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/port -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/tcp -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-app-2f-mqtt_client_app-2f-src

clean-Core-2f-app-2f-mqtt_client_app-2f-src:
	-$(RM) ./Core/app/mqtt_client_app/src/home_mqtt_gen.cyclo ./Core/app/mqtt_client_app/src/home_mqtt_gen.d ./Core/app/mqtt_client_app/src/home_mqtt_gen.o ./Core/app/mqtt_client_app/src/home_mqtt_gen.su ./Core/app/mqtt_client_app/src/mqtt.cyclo ./Core/app/mqtt_client_app/src/mqtt.d ./Core/app/mqtt_client_app/src/mqtt.o ./Core/app/mqtt_client_app/src/mqtt.su ./Core/app/mqtt_client_app/src/mqtt_gen_strings.cyclo ./Core/app/mqtt_client_app/src/mqtt_gen_strings.d ./Core/app/mqtt_client_app/src/mqtt_gen_strings.o ./Core/app/mqtt_client_app/src/mqtt_gen_strings.su

.PHONY: clean-Core-2f-app-2f-mqtt_client_app-2f-src

