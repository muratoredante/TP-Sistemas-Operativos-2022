################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/comunicacion.c \
../src/conexion.c \
../src/modulo_kernel.c \
../src/transicion.c 

OBJS += \
./src/comunicacion.o \
./src/conexion.o \
./src/modulo_kernel.o \
./src/transicion.o 

C_DEPS += \
./src/comunicacion.d \
./src/conexion.d \
./src/modulo_kernel.d \
./src/transicion.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2022-1c-Los-Picateclas-2.0/shared/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


