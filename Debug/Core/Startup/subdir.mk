################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f411ceux.s 

S_DEPS += \
./Core/Startup/startup_stm32f411ceux.d 

OBJS += \
./Core/Startup/startup_stm32f411ceux.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I../Core/app/mqtt_client_app/inc -I../Core/app/http_server_app/inc -I../Drivers/i2c_display/inc -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/mbedtls -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/library -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include/psa -I../Middlewares/Third_Party/crypto_lib_mbed_TLS/include -I../Core/app/sntp_app/inc -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=standard_c_nano_cpp.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f411ceux.d ./Core/Startup/startup_stm32f411ceux.o

.PHONY: clean-Core-2f-Startup

