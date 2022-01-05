#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t epc) {
  /*
  1. 将当前PC值保存到sepc寄存器
  2. 在scause寄存器中设置异常号
  3. 从stvec寄存器中取出异常入口地址
  4. 跳转到异常入口地址
  */
  csr_write(SEPC, epc);  
  csr_write(SCAUSE, NO); 
  interpret_rtl_j(csr_read(STVEC)); 
}

bool isa_query_intr(void) {
  return false;
}
