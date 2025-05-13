TCC_SRC_DIR := dependencies/tinycc
TCC_BUILD_DIR := build/tcc

TCC := $(TCC_BUILD_DIR)/bin/tcc

# Run our actual "compiler"
run: tcc cut gen
	$(TCC) -run main.c

# Cut out stencils
cut: tcc gen
	mkdir -p stencils
	$(TCC) -run cut.c

# Generate stencils
gen: tcc
	mkdir -p generated
	$(TCC) -run gen.c


# Build script and helper for tcc
tcc: $(TCC_BUILD_DIR)/bin/tcc

$(TCC_BUILD_DIR)/bin/tcc:
	@mkdir -p $(TCC_BUILD_DIR)
	cd $(TCC_SRC_DIR) && ./configure --prefix="$(abspath $(TCC_BUILD_DIR))"
	make -C $(TCC_SRC_DIR)
	make -C $(TCC_SRC_DIR) install


# Cleanup scripts :)
.PHONY: clean clean-tcc clean-gen clean-cut
clean: clean-tcc clean-gen clean-cut

clean-tcc:
	rm -rf $(TCC_BUILD_DIR)

clean-gen:
	rm -rf generated

clean-cut:
	rm -rf stencils


