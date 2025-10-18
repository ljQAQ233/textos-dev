ROOT = $(OUTPUT)/root
IMG  = $(OUTPUT)/image.img
export ROOT IMG

_img_m_:
	@echo "mounting disk..."
	mkdir -p $(MNT1)
	mkdir -p $(MNT2)
	sudo losetup -P $(LOOP) $(IMG)
	sudo mount $(LOOP)p1 $(MNT1)
	sudo mount $(LOOP)p2 $(MNT2)
	
_img_u_:
	@echo "unmounting disk..."
	sudo umount $(MNT1)
	sudo umount $(MNT2)
	sudo losetup -d $(LOOP)
	sudo rm -rf $(OUTPUT)/image

_img_cp_:
	sudo chmod a+rw $(MNT1)
	sudo chmod a+rw $(MNT2)
	cp -r resource/. $(ROOT)
	sudo cp -r $(ROOT)/. $(MNT1)
	sudo cp -r $(ROOT)/. $(MNT2)

_img_new_:
	dd if=/dev/zero of=$(IMG) bs=1M count=128
	sfdisk $(IMG) < utils/disk.conf
	sudo losetup -P $(LOOP) $(IMG)
	sudo mkfs.fat -F 32 -s 1 $(LOOP)p1
	sudo mkfs.minix -1 -n14 $(LOOP)p2
	sudo losetup -d $(LOOP)

# 更新策略: IMAGE_NEW 不为 false 时, 镜像将被重新 make, 原来的文件全部丢失.
# 若不生成 img, 每一次以 $(ROOT) 为主, 如果原来的镜像中有 与 root 冲突的部分
# root 的内容将覆盖原有内容. 如果 IMAGE_CP 为 false, 则镜像不会被更新.

ifneq (${IMAGE_CP},false)
  ifneq (${IMAGE_NEW},false)
    $(IMG): _img_new_ _img_m_ _img_cp_ _img_u_
  else
    $(IMG): _img_m_ _img_cp_ _img_u_
  endif
else
  $(IMG):
endif

LOOP:=$(shell sudo losetup -j ${IMG} | awk -F ':' '{print $$1}')
ifeq (${LOOP},)
  LOOP:=$(shell sudo losetup -f)
  diskmu: _img_m_
else
  diskmu: _img_u_
endif
MNT1:=$(OUTPUT)/image/$(notdir ${LOOP})p1
MNT2:=$(OUTPUT)/image/$(notdir ${LOOP})p2

$(ROOT):
	mkdir -p $@

pre: $(ROOT)

.PHONY: diskmu _img_m_ _img_u_ _img_cp_ _img_new_
