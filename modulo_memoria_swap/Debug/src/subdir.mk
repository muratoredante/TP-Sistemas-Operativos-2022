################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/espacio_kernel.c \
../src/espacio_usuario.c \
../src/modulo_memoria_swap.c 

OBJS += \
./src/espacio_kernel.o \
./src/espacio_usuario.o \
./src/modulo_memoria_swap.o 

C_DEPS += \
./src/espacio_kernel.d \
./src/espacio_usuario.d \
./src/modulo_memoria_swap.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2022-1c-Los-Picateclas-2.0/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


