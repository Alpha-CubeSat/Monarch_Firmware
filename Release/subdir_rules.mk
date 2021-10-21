################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/home/phoenix/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me -O2 --include_path="/home/phoenix/zztools/code_composer_studio/Monarch_Software" --include_path="/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/source/ti/posix/ccs" --include_path="/home/phoenix/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --define=DeviceFamily_CC13X0 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

build-340650415:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-340650415-inproc

build-340650415-inproc: ../hello.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"/home/phoenix/ti/xdctools_3_51_01_18_core/xs" --xdcpath="/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/source;/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "/home/phoenix/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me -O2 --include_path=\"/home/phoenix/zztools/code_composer_studio/Monarch_Software\" --include_path=\"/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/source/ti/posix/ccs\" --include_path=\"/home/phoenix/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include\" --define=DeviceFamily_CC13X0 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on  -std=c99 " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-340650415 ../hello.cfg
configPkg/compiler.opt: build-340650415
configPkg/: build-340650415


