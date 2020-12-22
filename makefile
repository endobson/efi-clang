
cc = clang
cflags = -I efi -target x86_64-pc-win32-coff -fno-stack-protector -fshort-wchar -mno-red-zone -Brepro \
         -fno-unwind-tables
ld = lld-link
lflags = -subsystem:efi_application -nodefaultlib -dll -timestamp:12345

all : hello-c.efi

hello-c.efi : hello-c.obj primitives.obj serial.obj
	$(ld) $(lflags) -entry:efi_main $^ -out:$@

hello-fasm.obj : hello-fasm.asm
	nasm -f win64 $^ -o hello-fasm.obj

hello-c.obj : hello-c.c primitives.h serial.h
	$(cc) $(cflags) -c $< -o $@

primitives.obj : primitives.asm primitives.h
	nasm -f win64 $< -o $@

serial.obj : serial.c serial.h
	$(cc) $(cflags) -c $< -o $@
	

.PHONY : clean
clean:
	if ls *.lib 1> /dev/null 2>&1 ; then rm *.lib ; fi
	if ls *.dll 1> /dev/null 2>&1 ; then rm *.dll ; fi
	if ls *.efi 1> /dev/null 2>&1 ; then rm *.efi ; fi
	if ls *.exe 1> /dev/null 2>&1 ; then rm *.exe ; fi
	if ls *.obj 1> /dev/null 2>&1 ; then rm *.obj ; fi
