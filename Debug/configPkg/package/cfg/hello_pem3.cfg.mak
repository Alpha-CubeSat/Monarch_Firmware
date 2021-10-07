# invoke SourceDir generated makefile for hello.pem3
hello.pem3: .libraries,hello.pem3
.libraries,hello.pem3: package/cfg/hello_pem3.xdl
	$(MAKE) -f /home/phoenix/zztools/code_composer_studio/Monarch_Software/src/makefile.libs

clean::
	$(MAKE) -f /home/phoenix/zztools/code_composer_studio/Monarch_Software/src/makefile.libs clean

