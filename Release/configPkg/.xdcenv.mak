#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/source;/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages
override XDCROOT = /home/phoenix/ti/xdctools_3_51_01_18_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/source;/home/phoenix/ti/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages;/home/phoenix/ti/xdctools_3_51_01_18_core/packages;..
HOSTOS = Linux
endif
