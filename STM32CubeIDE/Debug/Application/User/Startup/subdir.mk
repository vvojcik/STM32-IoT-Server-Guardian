################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/User/Startup/startup_stm32u585aiixq.s 

OBJS += \
./Application/User/Startup/startup_stm32u585aiixq.o 

S_DEPS += \
./Application/User/Startup/startup_stm32u585aiixq.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Startup/%.o: ../Application/User/Startup/%.s Application/User/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../../NetXDuo/App -I../../Core/Inc -I../../AZURE_RTOS/App -I../../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../../../../../../Middlewares/ST/netxduo/addons/dhcp -I../../../../../../../Middlewares/ST/threadx/common/inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../../Middlewares/ST/netxduo/common/inc -I../../../../../../../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I../../../../../../../Middlewares/ST/netxduo/addons/dns -I../../../../../../../Middlewares/ST/netxduo/addons/mqtt -I../../../../../../../Middlewares/ST/netxduo/addons/sntp -I../../../../../../../Middlewares/ST/netxduo/nx_secure/inc -I../../../../../../../Middlewares/ST/netxduo/nx_secure/ports -I../../../../../../../Middlewares/ST/netxduo/crypto_libraries/inc -I../../../../../../../Middlewares/ST/netxduo/crypto_libraries/ports/cortex_m4/gnu/inc -I../../../../../../../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/BSP/Components/mx_wifi -I../../../../../../../Middlewares/ST/netxduo/common/drivers/wifi/mxchip -I../../NetXDuo/Target -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Drivers/STM32U5xx_HAL_Driver/Inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/addons/dhcp -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/threadx/common/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Drivers/CMSIS/Device/ST/STM32U5xx/Include -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/common/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/addons/dns -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/addons/mqtt -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/addons/sntp -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/nx_secure/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/nx_secure/ports -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/crypto_libraries/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/netxduo/crypto_libraries/ports/cortex_m4/gnu/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -IC:/Users/admin/STM32Cube/Repository/STM32Cube_FW_U5_V1.8.0/Drivers/CMSIS/Include -I../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../../Middlewares/ST/netxduo/addons/dhcp -I../../Middlewares/ST/threadx/common/inc -I../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../Middlewares/ST/netxduo/common/inc -I../../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I../../Middlewares/ST/netxduo/addons/dns -I../../Middlewares/ST/netxduo/addons/mqtt -I../../Middlewares/ST/netxduo/addons/sntp -I../../Middlewares/ST/netxduo/nx_secure/inc -I../../Middlewares/ST/netxduo/nx_secure/ports -I../../Middlewares/ST/netxduo/crypto_libraries/inc -I../../Middlewares/ST/netxduo/crypto_libraries/ports/cortex_m4/gnu/inc -I../../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../../Drivers/CMSIS/Include -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Application-2f-User-2f-Startup

clean-Application-2f-User-2f-Startup:
	-$(RM) ./Application/User/Startup/startup_stm32u585aiixq.d ./Application/User/Startup/startup_stm32u585aiixq.o

.PHONY: clean-Application-2f-User-2f-Startup

