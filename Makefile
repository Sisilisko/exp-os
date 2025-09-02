
.PHONY: kernel mods limine disk clean


kernel:
	@$(MAKE) -C kernel/ all


mkiso:
	sh ./scripts/createiso.sh


clean:
	@$(MAKE) -C kernel/ clean

