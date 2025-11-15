################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Src/osKernelAssembly.s 

C_SRCS += \
../Src/adc1.c \
../Src/gpio_out.c \
../Src/main.c \
../Src/osKernel.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/uart.c 

OBJS += \
./Src/adc1.o \
./Src/gpio_out.o \
./Src/main.o \
./Src/osKernel.o \
./Src/osKernelAssembly.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/uart.o 

S_DEPS += \
./Src/osKernelAssembly.d 

C_DEPS += \
./Src/adc1.d \
./Src/gpio_out.d \
./Src/main.d \
./Src/osKernel.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -DSTM32F401xC -c -I../Inc -I"C:/Users/lfnin/OneDrive/Documents/Studies/Embedded Systems/8. Embedded_Expert_IO/5. Phase1/6. Milestones/P2_RTOS_Kernel/F4_chip_headers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/lfnin/OneDrive/Documents/Studies/Embedded Systems/8. Embedded_Expert_IO/5. Phase1/6. Milestones/P2_RTOS_Kernel/F4_chip_headers/CMSIS/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.s Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/adc1.cyclo ./Src/adc1.d ./Src/adc1.o ./Src/adc1.su ./Src/gpio_out.cyclo ./Src/gpio_out.d ./Src/gpio_out.o ./Src/gpio_out.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/osKernel.cyclo ./Src/osKernel.d ./Src/osKernel.o ./Src/osKernel.su ./Src/osKernelAssembly.d ./Src/osKernelAssembly.o ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/uart.cyclo ./Src/uart.d ./Src/uart.o ./Src/uart.su

.PHONY: clean-Src

