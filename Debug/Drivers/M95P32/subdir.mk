################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/M95P32/m95p32.c 

OBJS += \
./Drivers/M95P32/m95p32.o 

C_DEPS += \
./Drivers/M95P32/m95p32.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/M95P32/%.o Drivers/M95P32/%.su Drivers/M95P32/%.cyclo: ../Drivers/M95P32/%.c Drivers/M95P32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/smpet/Desktop/Github_BMS_Master/26_BMS_Master/Drivers/LTC6813/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/smpet/Desktop/26-BMS_Master/Drivers/Elcon" -I"C:/Users/smpet/Desktop/Github_BMS_Master/26_BMS_Master/Drivers/Elcon" -I"C:/Users/smpet/Desktop/Github_BMS_Master/26_BMS_Master/Drivers/M95P32" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-M95P32

clean-Drivers-2f-M95P32:
	-$(RM) ./Drivers/M95P32/m95p32.cyclo ./Drivers/M95P32/m95p32.d ./Drivers/M95P32/m95p32.o ./Drivers/M95P32/m95p32.su

.PHONY: clean-Drivers-2f-M95P32

