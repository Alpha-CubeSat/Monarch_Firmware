################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/home/wpc/UserSoftware/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me -O2 --include_path="/home/wpc/projects/CCS_workspace/Sprite" --include_path="/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages/ti/sysbios/posix" --include_path="/home/wpc/UserSoftware/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --define=DeviceFamily_CC13X0 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1196772191:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1196772191-inproc

build-1196772191-inproc: ../hello.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"/home/wpc/UserSoftware/CCS/xdctools_3_50_08_24_core/xs" --xdcpath="/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/source;/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages;/home/wpc/UserSoftware/CCS/ccsv8/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "/home/wpc/UserSoftware/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me -O2 --include_path=\"/home/wpc/projects/CCS_workspace/Sprite\" --include_path=\"/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages/ti/sysbios/posix\" --include_path=\"/home/wpc/UserSoftware/CCS/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include\" --define=DeviceFamily_CC13X0 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1196772191 ../hello.cfg
configPkg/compiler.opt: build-1196772191
configPkg/: build-1196772191


