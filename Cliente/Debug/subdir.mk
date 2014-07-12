################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../CommunicationSocket.o \
../cliente_1.0.o 

CPP_SRCS += \
../cliente_1.0\ (un\ mismo\ socket\ de\ partida).cpp \
../cliente_1.0.cpp 

OBJS += \
./cliente_1.0\ (un\ mismo\ socket\ de\ partida).o \
./cliente_1.0.o 

CPP_DEPS += \
./cliente_1.0\ (un\ mismo\ socket\ de\ partida).d \
./cliente_1.0.d 


# Each subdirectory must supply rules for building sources it contributes
cliente_1.0\ (un\ mismo\ socket\ de\ partida).o: ../cliente_1.0\ (un\ mismo\ socket\ de\ partida).cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"cliente_1.0 (un mismo socket de partida).d" -MT"cliente_1.0\ (un\ mismo\ socket\ de\ partida).d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


