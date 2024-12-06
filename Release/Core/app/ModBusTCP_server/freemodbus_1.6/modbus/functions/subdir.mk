################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.c \
../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.c 

C_DEPS += \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.d \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.d 

OBJS += \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.o \
./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.o 


# Each subdirectory must supply rules for building sources it contributes
Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/%.o Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/%.su Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/%.cyclo: ../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/%.c Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/LittleFS/inc -I../Middlewares/Third_Party/mongoose -I../Drivers/SPI_FLASH/inc -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Middlewares/Third_Party/validation/inc -I../Drivers/i2c_display/inc -I../Core/app/sntp_app/inc -I../Core/app/ssdp_auto_discovery_app/inc -I../Core/app/ModBusTCP_server/inc -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/include -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/port -I../Core/app/ModBusTCP_server/freemodbus_1.6/modbus/tcp -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-app-2f-ModBusTCP_server-2f-freemodbus_1-2e-6-2f-modbus-2f-functions

clean-Core-2f-app-2f-ModBusTCP_server-2f-freemodbus_1-2e-6-2f-modbus-2f-functions:
	-$(RM) ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfunccoils.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdiag.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncdisc.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncholding.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncinput.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbfuncother.su ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.cyclo ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.d ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.o ./Core/app/ModBusTCP_server/freemodbus_1.6/modbus/functions/mbutils.su

.PHONY: clean-Core-2f-app-2f-ModBusTCP_server-2f-freemodbus_1-2e-6-2f-modbus-2f-functions

