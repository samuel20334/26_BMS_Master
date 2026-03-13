################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/LTC6813/bms_hardware.c \
../Drivers/LTC6813/ltc6813.c \
../Drivers/LTC6813/ltc681x.c 

OBJS += \
./Drivers/LTC6813/bms_hardware.o \
./Drivers/LTC6813/ltc6813.o \
./Drivers/LTC6813/ltc681x.o 

C_DEPS += \
./Drivers/LTC6813/bms_hardware.d \
./Drivers/LTC6813/ltc6813.d \
./Drivers/LTC6813/ltc681x.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/LTC6813/%.o Drivers/LTC6813/%.su Drivers/LTC6813/%.cyclo: ../Drivers/LTC6813/%.c Drivers/LTC6813/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-LTC6813

clean-Drivers-2f-LTC6813:
	-$(RM) ./Drivers/LTC6813/bms_hardware.cyclo ./Drivers/LTC6813/bms_hardware.d ./Drivers/LTC6813/bms_hardware.o ./Drivers/LTC6813/bms_hardware.su ./Drivers/LTC6813/ltc6813.cyclo ./Drivers/LTC6813/ltc6813.d ./Drivers/LTC6813/ltc6813.o ./Drivers/LTC6813/ltc6813.su ./Drivers/LTC6813/ltc681x.cyclo ./Drivers/LTC6813/ltc681x.d ./Drivers/LTC6813/ltc681x.o ./Drivers/LTC6813/ltc681x.su

.PHONY: clean-Drivers-2f-LTC6813

