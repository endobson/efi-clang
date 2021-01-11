
cc = clang
cflags = -target x86_64-pc-win32-coff -fno-stack-protector -fshort-wchar -mno-red-zone -Brepro \
         -fno-unwind-tables
ld = lld-link
lflags = -subsystem:efi_application -nodefaultlib -dll -timestamp:12345

all : hello-c.efi

hello-c.efi : hello-c.obj primitives.obj serial.obj strings.obj efi_util.obj \
              descriptor_tables.obj scheduler.obj examples.obj msabi-runtime.obj
	$(ld) $(lflags) -entry:efi_main $^ -out:$@

hello-c.obj : hello-c.c primitives.h serial.h strings.h efi_util.h acpi.h scheduler.h
	$(cc) $(cflags) -c $< -o $@

primitives.obj : primitives.asm primitives.h
	nasm -f win64 $< -o $@

serial.obj : serial.c serial.h
	$(cc) $(cflags) -c $< -o $@

strings.obj : strings.c strings.h
	$(cc) $(cflags) -c $< -o $@

efi_util.obj : efi_util.c efi_util.h strings.h
	$(cc) $(cflags) -c $< -o $@

descriptor_tables.obj : descriptor_tables.c descriptor_tables.h primitives.h
	$(cc) $(cflags) -c $< -o $@

scheduler.obj : scheduler.c scheduler.h
	$(cc) $(cflags) -c $< -o $@
	

.PHONY : clean
clean:
	if ls *.lib 1> /dev/null 2>&1 ; then rm *.lib ; fi
	if ls *.dll 1> /dev/null 2>&1 ; then rm *.dll ; fi
	if ls *.efi 1> /dev/null 2>&1 ; then rm *.efi ; fi
	if ls *.exe 1> /dev/null 2>&1 ; then rm *.exe ; fi
	if ls *.obj 1> /dev/null 2>&1 ; then rm *.obj ; fi
