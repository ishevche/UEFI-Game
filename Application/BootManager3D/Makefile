PROJECT_PATH = Wordle
IMAGE_PATH = drive-image.fat
GPT_IMAGE_PATH = drive_image.img
IMAGE_CONTENT_DIR = drive-image

MODULEFILE = $(PROJECT_PATH)/Application/BootManager3D/BootManager3D.inf
TOOL_CHAIN_CONF = Conf/tools_def.txt
CODE = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_CODE.fd
VARS = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_VARS.fd
EXECUTABLE_LOCATION = ${WORKSPACE}/Build/OSProject/DEBUG_GCC/X64
EXECUTABLE_NAME = BootManager3D.efi

all: qemu

content-dir:
	mkdir -p $(IMAGE_CONTENT_DIR)/EFI

image:
	dd if=/dev/zero of=$(IMAGE_PATH) count=1 bs=40M

formatted-image: image
	mkfs.fat -F32 $(IMAGE_PATH)

build:
	build -m $(MODULEFILE)

copy-executable: build content-dir
	cp $(EXECUTABLE_LOCATION)/$(EXECUTABLE_NAME) $(IMAGE_CONTENT_DIR)/EFI

populated-image: formatted-image copy-executable
	mcopy -vosQ -i $(IMAGE_PATH) $(IMAGE_CONTENT_DIR)/* ::

copied-image: populated-image
	sudo kpartx -av $(GPT_IMAGE_PATH)
	sudo dd if=$(IMAGE_PATH) of=/dev/mapper/loop0p4 bs=40M
	sudo kpartx -dv $(GPT_IMAGE_PATH)

qemu: copied-image
	qemu-system-x86_64 \
	  -enable-kvm \
      -drive format=raw,file=$(GPT_IMAGE_PATH) \
      -drive if=pflash,format=raw,readonly=on,file=$(CODE) \
      -drive if=pflash,format=raw,file=$(VARS) \
      -net none -m 2G
#      -cdrom $(WORKSPACE)/Wordle/ubuntu-22.04.3-desktop-amd64.iso \
#      -nographic \
