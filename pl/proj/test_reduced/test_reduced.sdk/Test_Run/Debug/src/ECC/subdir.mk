################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ECC/uECC.c 

OBJS += \
./src/ECC/uECC.o 

C_DEPS += \
./src/ECC/uECC.d 


# Each subdirectory must supply rules for building sources it contributes
src/ECC/%.o: ../src/ECC/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../Test_Run_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


