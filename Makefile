
.PHONY: kernel mods limine disk clean


kernel:
	@$(MAKE) -C kernel/ all

clean:
	@$(MAKE) -C kernel/ clean
