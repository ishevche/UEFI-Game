PROJECT_PATH = Wordle
IMAGE_PATH = drive-image1.fat
IMAGE_CONTENT_DIR = drive-image

MODULEFILE = $(PROJECT_PATH)/Application/XV6/XV6.inf
XV6_SOURCE_DIR = $(PROJECT_PATH)/Application/XV6/data
TOOL_CHAIN_CONF = Conf/tools_def.txt
CODE = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_CODE.fd
VARS = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_VARS.fd
EXECUTABLE_LOCATION = ${WORKSPACE}/Build/OSProject/DEBUG_GCC/X64
EXECUTABLE_NAME = XV6.*

ifndef QEMU
QEMU = $(shell if which qemu > /dev/null; \
	then echo qemu; exit; \
	elif which qemu-system-x86_64 > /dev/null; \
	then echo qemu-system-x86_64; exit; \
	elif which qemu-system-i386 > /dev/null; \
	then echo qemu-system-i386; exit; \
	else \
	qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "*** or have you tried setting the QEMU variable in Makefile?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)


all: qemu

content-dir:
	mkdir -p $(IMAGE_CONTENT_DIR)/EFI

image:
	dd if=/dev/zero of=$(IMAGE_PATH) count=1 bs=50M

formatted-image: image
	mkfs.fat -F32 $(IMAGE_PATH)

build:
	build -m $(MODULEFILE)

copy-executable: build content-dir
	cp $(EXECUTABLE_LOCATION)/$(EXECUTABLE_NAME) $(IMAGE_CONTENT_DIR)/EFI

kernel:
	cd $(WORKSPACE)/$(XV6_SOURCE_DIR) && make kernel fs.img
	cp $(WORKSPACE)/$(XV6_SOURCE_DIR)/kernel $(IMAGE_CONTENT_DIR)/kernel
	cp $(WORKSPACE)/$(XV6_SOURCE_DIR)/fs.img xv6_fs.img

populated-image: formatted-image copy-executable kernel
	mcopy -vosQ -i $(IMAGE_PATH) $(IMAGE_CONTENT_DIR)/* ::

qemu: populated-image
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) \
	  -enable-kvm \
      -drive if=pflash,format=raw,readonly=on,file=${CODE} \
      -drive if=pflash,format=raw,file=${VARS} \
      -drive format=raw,file=${IMAGE_PATH} \
      -drive format=raw,file=xv6_fs.img \
      -net none -nographic -m 2048 \
      -S $(QEMUGDB)


.PHONY: kernel build