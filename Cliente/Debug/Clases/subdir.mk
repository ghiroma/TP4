################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Clases/CommunicationSocket.cpp \
../Clases/Hilo.cpp \
../Clases/Semaforo.cpp \
../Clases/ServerSocket.cpp 

OBJS += \
./Clases/CommunicationSocket.o \
./Clases/Hilo.o \
./Clases/Semaforo.o \
./Clases/ServerSocket.o 

CPP_DEPS += \
./Clases/CommunicationSocket.d \
./Clases/Hilo.d \
./Clases/Semaforo.d \
./Clases/ServerSocket.d 


# Each subdirectory must supply rules for building sources it contributes
Clases/%.o: ../Clases/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


