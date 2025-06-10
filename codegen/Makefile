
.PHONY: cut codegen

cc=gcc -fno-toplevel-reorder -fno-align-functions
codegen:
	$(cc) -g -O1 -I../ codegen.c print_c_code.c ../ast/*.c -o codegen
	@./codegen

cut: codegen
	$(cc) -g -O2 -I../ cut.c ../ast/*.c -o cut
	@./cut

