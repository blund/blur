#include "stdio.h"

#define STB_DS_ASSERT assert
#define STB_DS_IMPLEMENTATION
#include "../include/stb_ds.h"

#include "../ast/ast.h"
#include "../ast/build.h"

#include "cps.h"
#include "print.h"
#include "transform.h"

Block *example_ast();

int main() {
  Block*   b = example_ast();
  CpsNode *n = transform_ast(b);

  print_cps_graph(n);
}

Block* example_ast() {
  return new_block(new_assign("test", new_integer(3)),
                   new_if(new_call_expr("leq", (Arguments){
			 .count = 2,
			 .entries = {new_integer(1),
				     new_integer(4)}}),
		     new_block(new_call(
					"add", (Arguments){.count = 2,
							   .entries = {new_identifier("test"),
								       new_integer(4)}})),
		     new_block(new_call(
					"add", (Arguments){.count = 2,
							   .entries = {new_identifier("test"),
								       new_integer(7)}}))));
}

