
cc=gcc -fno-toplevel-reorder -fno-align-functions
codegen:
	$(cc) -g -O1 -I../ codegen.c print_c_code.c ../ast/*.c -lm -o blur
	@./blur

cut:
	$(cc) -g -O2 -I../ cut.c ../ast/*.c -DDEBUG -o cut-test
	@./cut-test

read:
	$(cc) -g -I../ read.c ../ast/*.c ../copy_and_patch.c -lm -o read-test
	@./read-test
