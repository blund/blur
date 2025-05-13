TCC_SRC_DIR := dependencies/tinycc
TCC_BUILD_DIR := build/tcc

TCC := $(TCC_INSTALL_DIR)/bin/tcc

# Run the "compiler"
run: tcc
	$(TCC) -run add.c && $(TCC) -run main.c


# Configure, build, and install TCC
tcc: $(TCC_INSTALL_DIR)/bin/tcc

$(TCC_INSTALL_DIR)/bin/tcc:
	@mkdir -p $(TCC_BUILD_DIR)
	cd $(TCC_SRC_DIR) && ./configure --prefix="$(abspath $(TCC_BUILD_DIR))"
	make -C $(TCC_SRC_DIR)
	make -C $(TCC_SRC_DIR) install

# Clean TCC build
.PHONY: clean-tcc
clean-tcc:
	rm -rf $(TCC_BUILD_DIR)

# Clean everything
.PHONY: clean
clean: clean-tcc
