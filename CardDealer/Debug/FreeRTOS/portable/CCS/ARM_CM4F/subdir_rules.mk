################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/CCS/ARM_CM4F/%.obj: ../FreeRTOS/portable/CCS/ARM_CM4F/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/FreeRTOS/include" --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/LcdDriver" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer/Libraries" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer/Libraries/DefaultLib" --include_path="C:/ti/ccs1210/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1210/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer" --include_path="C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/FreeRTOS/portable/CCS/ARM_CM4F" --include_path="C:/Users/Francesco Olivieri/Documents/simplelink_msp432p4_sdk_3_40_01_02/simplelink_msp432p4_sdk_3_40_01_02/source" --advice:power="all" --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/portable/CCS/ARM_CM4F/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS/portable/CCS/ARM_CM4F" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

FreeRTOS/portable/CCS/ARM_CM4F/%.obj: ../FreeRTOS/portable/CCS/ARM_CM4F/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/FreeRTOS/include" --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/LcdDriver" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer/Libraries" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer/Libraries/DefaultLib" --include_path="C:/ti/ccs1210/ccs/ccs_base/arm/include" --include_path="C:/ti/ccs1210/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Francesco Olivieri/Documents/GitHub/Card-Dealer/CardDealer" --include_path="C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="C:/Users/Francesco Olivieri/Desktop/UNI/SW EMBEDDED SYSTEMS/CardDealer/FreeRTOS/portable/CCS/ARM_CM4F" --include_path="C:/Users/Francesco Olivieri/Documents/simplelink_msp432p4_sdk_3_40_01_02/simplelink_msp432p4_sdk_3_40_01_02/source" --advice:power="all" --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/portable/CCS/ARM_CM4F/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS/portable/CCS/ARM_CM4F" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


