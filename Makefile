.PHONY: cut gen

# This should work with most C compilers, test at your own pleasure :)
cc=zig cc
# cc=tcc
# cc=gcc -fno-toplevel-reorder
# cc=clang

# Enable debug printing for information about stencil building and cutting
debug=-DDEBUG

# Run our actual "compiler"
run: cut
	@$(cc) $(debug) -O2 -g main.c -o blur
	@./blur

# Cut out stencils
cut: gen
	@mkdir -p generated/stencils
	@$(cc) $(debug) -O2 cut.c -o cut 
	@./cut

# Generate stencils
gen:
	@mkdir -p generated
	@$(cc) $(debug) gen.c pond/build.c pond/print.c -o gen
	@./gen

# Cleanup scripts :)
.PHONY: clean clean-blur clean-gen clean-cut
clean: clean-gen clean-cut

clean-blur:
	rm blur

clean-gen:
	rm -rf generated
	rm gen

clean-cut:
	rm -rf stencils
	rm cut


