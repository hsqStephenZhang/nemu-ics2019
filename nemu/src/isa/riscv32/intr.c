#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t epc) {
  csr_write(SEPC, epc);  
  csr_write(SCAUSE, NO); 
  interpret_rtl_j(csr_read(STVEC)); 
}

bool isa_query_intr(void) {
  return false;
}
