################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AES/aes.c \
../src/AES/aes_decrypt_driver.c 

OBJS += \
./src/AES/aes.o \
./src/AES/aes_decrypt_driver.o 

C_DEPS += \
./src/AES/aes.d \
./src/AES/aes_decrypt_driver.d 


# Each subdirectory must supply rules for building sources it contributes
src/AES/%.o: ../src/AES/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../Test_Run_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


