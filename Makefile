PROJECT_PATH = Wordle
IMAGE_PATH = drive-image.fat
IMAGE_CONTENT_DIR = drive-image

MODULEFILE = $(PROJECT_PATH)/Application/Wordle/Wordle.inf
TOOL_CHAIN_CONF = Conf/tools_def.txt
CODE = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_CODE.fd
VARS = ${WORKSPACE}/Build/OvmfX64/DEBUG_GCC/FV/OVMF_VARS.fd
EXECUTABLE_LOCATION = ${WORKSPACE}/Build/OSProject/DEBUG_GCC/X64
EXECUTABLE_NAME = Wordle.efi

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

populated-image: formatted-image copy-executable
	mcopy -vosQ -i $(IMAGE_PATH) $(IMAGE_CONTENT_DIR)/* ::

qemu: populated-image
	qemu-system-x86_64 \
      -drive if=pflash,format=raw,readonly=on,file=${CODE} \
      -drive if=pflash,format=raw,file=${VARS} \
      -drive format=raw,file=${IMAGE_PATH} \
      -nographic -net none