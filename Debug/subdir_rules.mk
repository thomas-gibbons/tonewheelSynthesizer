################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCStudio_v8/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmsp -O0 --use_hw_mpy=none --include_path="C:/CCStudio_v8/ccsv8/ccs_base/msp430/include" --include_path="C:/CCStudio_v8/ccsv8/ccs_base/msp430/include" --include_path="C:/CCStudio_v8/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --define=__MSP430F2272__ -g --printf_support=full --diag_warning=225 --abi=eabi --silicon_errata=CPU19 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


