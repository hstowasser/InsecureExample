################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/HASH/md5.c \
../src/HASH/sha1.c \
../src/HASH/sha256.c 

OBJS += \
./src/HASH/md5.o \
./src/HASH/sha1.o \
./src/HASH/sha256.o 

C_DEPS += \
./src/HASH/md5.d \
./src/HASH/sha1.d \
./src/HASH/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
src/HASH/%.o: ../src/HASH/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../Test_Run_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


