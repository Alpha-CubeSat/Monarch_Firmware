#
_XDCBUILDCOUNT = 0
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/source;/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages
override XDCROOT = /home/wpc/UserSoftware/CCS/xdctools_3_50_08_24_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/source;/home/wpc/UserSoftware/CCS/simplelink_cc13x0_sdk_2_30_00_20/kernel/tirtos/packages;/home/wpc/UserSoftware/CCS/xdctools_3_50_08_24_core/packages;..
HOSTOS = Linux
endif
