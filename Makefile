.PHONY: cut gen

CC=zig cc
# CC=gcc -fno-toplevel-reorder

# Run our actual "compiler"
run: cut
	$(CC) -O2 -g main.c -o blur
	@./blur

# Cut out stencils
cut: gen
	@mkdir -p generated/stencils
	$(CC) -O2 cut.c -o cut
	@./cut

# Generate stencils
gen:
	@mkdir -p generated
	$(CC) gen.c pond/parse.c pond/print.c -o gen
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


