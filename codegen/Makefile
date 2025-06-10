
cc=gcc -fno-toplevel-reorder -fno-align-functions

generate-stencils:
	@mkdir -p ../generated/
	$(cc) -g -O1 -I../ codegen.c print_c_code.c ../ast/*.c -o codegen
	@./codegen

cut-stencils:
	$(cc) -g -O2 -I../ cut.c ../ast/*.c -o cut
	@./cut

