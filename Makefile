.PHONY: cut gen

# Run our actual "compiler"
run: cut gen
	gcc main.c -o blur
	@./blur

# Cut out stencils
cut:
	@mkdir -p generated/stencils
	gcc -O2 -fno-toplevel-reorder -fno-align-functions cut.c -o cut
	@./cut

# Generate stencils
gen:
	@mkdir -p generated
	gcc gen.c

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


