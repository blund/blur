.PHONY: gen

# This should work with most C compilers, test at your own pleasure :)
cc=gcc -fno-toplevel-reorder -fno-align-functions

# Enable debug printing for information about stencil building and cutting
# debug=-DDEBUG

# Run our actual "compiler"
run:
	@$(cc) -g -O2 -I. main.c copy_and_patch/*.c ast/*.c ir/*.c -lm -o blur
	@./blur

gen:
	make -C codegen generate-stencils
	make -C codegen cut-stencils

# Cleanup scripts :)
.PHONY: clean clean-blur clean-gen clean-cut
clean: clean-gen clean-cut

clean-blur:
	rm blur

clean-gen:
	rm -rf generated
