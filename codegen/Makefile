
cc=gcc -fno-toplevel-reorder -fno-align-functions
.PHONY: cut codegen

codegen:
	$(cc) -g -O1 -I../ codegen.c print_c_code.c ../ast/*.c -lm -o blur
	@./blur

cut: codegen
	$(cc) -g -O1 -I../ cut.c ../ast/*.c -DDEBUG -o cut-test
	@./cut-test

read: cut
	$(cc) -DDEBUG -g -I../ read.c ../ast/*.c  ../ir/*.c ../copy_and_patch.c -lm -o read-test
	@./read-test
