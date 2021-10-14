# invoke SourceDir generated makefile for hello.pem3
hello.pem3: .libraries,hello.pem3
.libraries,hello.pem3: package/cfg/hello_pem3.xdl
	$(MAKE) -f /home/wpc/projects/CCS_workspace/Monarch_Firmware/src/makefile.libs

clean::
	$(MAKE) -f /home/wpc/projects/CCS_workspace/Monarch_Firmware/src/makefile.libs clean

