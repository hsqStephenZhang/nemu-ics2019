#include "cpu/exec.h"
#include "all-instr.h"

/**
 * IDEX(ld, load)
 * according to 14-12 bit, there are 5 conditions
 * lb  000 
 * lh  001
 * lw  010
 *  -  011
 * lbu 100
 * lhu 101
 */
static OpcodeEntry load_table [8] = {
  EXW(lb_lh, 1), EXW(lb_lh, 2), EXW(ld, 4), EMPTY, EXW(ld, 1), EXW(ld, 2), EMPTY, EMPTY
};

static make_EHelper(load) {
  decinfo.width = load_table[decinfo.isa.instr.funct3].width;
  idex(pc, &load_table[decinfo.isa.instr.funct3]);
}
/**
 * sb 000
 * sh 001
 * sw 010
 */ 
static OpcodeEntry store_table [8] = {
  EXW(st, 1), EXW(st, 2), EXW(st, 4), EMPTY, EMPTY, EMPTY, EMPTY, EMPTY
};

static make_EHelper(store) {
  decinfo.width = store_table[decinfo.isa.instr.funct3].width;
  idex(pc, &store_table[decinfo.isa.instr.funct3]);
}

/**
 * addi  000
 * slli  001
 * slti  010
 * sltiu 011
 * xori  100
 * 
 * srli  101
 * srai  101
 * 
 * ori   110
 * andi  111
 */

static OpcodeEntry imm_table [8] = {
  EX(addi), EX(slli), EX(slti), EX(sltiu), EX(xori), EX(sr_li_ai), EX(ori), EX(andi),
};


static make_EHelper(imm) {
  decinfo.width = imm_table[decinfo.isa.instr.funct3].width;
  idex(pc, &imm_table[decinfo.isa.instr.funct3]);
}

/**
 * add, sub 000
 * sll      001
 * slt      010
 * sltu     011
 * xor      100
 * srl, sra 101
 * or       110
 * and      111
 */
static OpcodeEntry r_table [8] = {
  EX(add_sub), EX(sll), EX(slt), EX(sltu), EX(xor), EX(slr_sra), EX(or), EX(and)
};

/** R
 * mul    000
 * mulh   001
 * mulhsu 010
 * mulhu  011
 * div    100
 * divu   101
 * rem    110
 * remu   111
 */
static OpcodeEntry r_m_table[8] = {
  EX(mul), EX(mulh), EX(mulhsu), EX(mulhu), EX(div), EX(divu), EX(rem), EX(remu)
};

static make_EHelper(r) {
  decinfo.width = imm_table[decinfo.isa.instr.funct3].width;
  decinfo.isa.instr.funct7 == 0b0000001 ? 
  idex(pc, &r_m_table[decinfo.isa.instr.funct3]) :
  idex(pc, &r_table[decinfo.isa.instr.funct3]);
};

static OpcodeEntry system_table[8] = {
  EX(ECALL_EBREAK), EX(CSRRW), EX(CSRRS), EX(CSRRC), EMPTY, EX(CSRRWI), EX(CSSRRSI), EX(CSRRCI)
};

static make_EHelper(system) { /* static void exec_system(vaddr_T *pc) */
  //decinfo.width = system_table[decinfo.isa.instr.funct3].width;
  idex(pc, &system_table[decinfo.isa.instr.funct3]);
}


static OpcodeEntry opcode_table [32] = {
  /* b00 */ IDEX(ld, load), EMPTY, EMPTY, EMPTY, IDEX(I, imm), IDEX(U, auipc), EMPTY, EMPTY,
  /* b01 */ IDEX(st, store), EMPTY, EMPTY, EMPTY, IDEX(R, r), IDEX(U, lui), EMPTY, EMPTY,
  /* b10 */ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  /* b11 */ IDEX(B, branch), IDEX(I, jalr), EX(nemu_trap), IDEX(J, jal), IDEX(I, system), EMPTY, EMPTY, EMPTY,
};

void isa_exec(vaddr_t *pc) {
  decinfo.isa.instr.val = instr_fetch(pc, 4);
  assert(decinfo.isa.instr.opcode1_0 == 0x3);
  idex(pc, &opcode_table[decinfo.isa.instr.opcode6_2]);
}