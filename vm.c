#include "vm.h"

int vm_init(VM * vm)
{
	vm->op_sp   = 0;
	vm->call_sp = 0;
	vm->insts   = NULL;
}

void vm_test()
{
	
}

