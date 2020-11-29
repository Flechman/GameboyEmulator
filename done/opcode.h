#pragma once

#include "bit.h" // CLAMP07, bit_get
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//=========================================================================
typedef uint8_t opcode_t;

typedef enum {
    DIRECT = 0x00, PREFIXED = 0xCB
} opcode_kind;

// ======================================================================
/**
 * @brief Check opcodes. To be launched once for all at the beginning of your
 *        program if you want to be sure no bug was introduced in the opcode
 *        representation
 *
 * @return 1 if ok, 0 if not.
 */
int opcode_check_integrity();

// ======================================================================
/**
 * @brief Extract register from OPCODE
 */
#define OPCODE_REG_MASK 0x7
#define extract_reg(op, index) (((op) >> CLAMP07(index)) & OPCODE_REG_MASK)
#define extract_reg_at_zero(op) extract_reg(op, 0)
#define extract_reg_at_three(op) extract_reg(op, 3)

// ======================================================================
/**
 * @brief Extract register pair from OPCODE
 */
#define OPCODE_REG_PAIR_IDX 4
#define OPCODE_REG_PAIR_MASK 0x3
#define extract_reg_pair(op) (extract_reg(op, OPCODE_REG_PAIR_IDX) & OPCODE_REG_PAIR_MASK)

// ======================================================================
/**
* @brief Macro to extract N3 from OPCODE
*/
#define N3_IDX 3
#define N3_MASK 0x7
#define extract_n3(op) (((op) >> N3_IDX) & N3_MASK)

// ======================================================================
/**
 * @brief Instruction families
 *
 */
typedef enum {
    NOP,

    // Load
    LD_A_BCR,
    LD_A_CR,
    LD_A_DER,
    LD_A_HLRU,
    LD_A_N16R,
    LD_A_N8R,
    LD_R16SP_N16,
    LD_R8_HLR,
    LD_R8_N8,
    POP_R16,

    // Store
    LD_BCR_A,
    LD_CR_A,
    LD_DER_A,
    LD_HLRU_A,
    LD_HLR_N8,
    LD_HLR_R8,
    LD_N16R_A,
    LD_N16R_SP,
    LD_N8R_A,
    PUSH_R16,

    // Move
    LD_R8_R8,
    LD_SP_HL,

    // Add
    ADD_A_HLR,
    ADD_A_N8,
    ADD_A_R8,
    ADD_HL_R16SP,
    INC_HLR,
    INC_R16SP,
    INC_R8,
    LD_HLSP_S8,

    // Subtract / compare
    CP_A_HLR,
    CP_A_N8,
    CP_A_R8,
    DEC_HLR,
    DEC_R16SP,
    DEC_R8,
    SUB_A_HLR,
    SUB_A_N8,
    SUB_A_R8,

    // And
    AND_A_HLR,
    AND_A_N8,
    AND_A_R8,

    // (Inclusive) or
    OR_A_HLR,
    OR_A_N8,
    OR_A_R8,

    // Exclusive or
    XOR_A_HLR,
    XOR_A_N8,
    XOR_A_R8,

    // Rotate
    ROTA,
    ROTCA,
    ROTC_HLR,
    ROTC_R8,
    ROT_HLR,
    ROT_R8,
    SWAP_HLR,
    SWAP_R8,

    // Shift
    SLA_HLR,
    SLA_R8,
    SRA_HLR,
    SRA_R8,
    SRL_HLR,
    SRL_R8,

    // Bit test and (re)set
    BIT_U3_HLR,
    BIT_U3_R8,
    CHG_U3_HLR,
    CHG_U3_R8,

    // Miscellaneous ALU operations
    CPL,
    DAA,
    SCCF,

    // Jumps
    JP_CC_N16,
    JP_HL,
    JP_N16,
    JR_CC_E8,
    JR_E8,

    // Procedure call and return
    CALL_CC_N16,
    CALL_N16,
    RET,
    RET_CC,
    RST_U3,

    // Interrupt handling
    EDI,
    RETI,

    // Miscellaneous control instructions
    HALT,
    STOP,

    // Unknown opcode
    UNKN
} opcode_family;


//=========================================================================
/**
 * @brief Type to represent Instruction
 */
typedef struct {
    opcode_kind kind;
    opcode_family family;
    opcode_t opcode;
    uint8_t bytes; //1, 2 or 3
    uint8_t cycles;
    uint8_t xtra_cycles;
} instruction_t;


// ======================================================================
#define INSTR_DFX(Kind, Family, Code, Bytes, Cycles, Xtra) \
  { .kind=Kind, .family=Family, .opcode=Code, .bytes=Bytes, .cycles=Cycles, .xtra_cycles=Xtra }
#define INSTR_DEF(Kind, Family, Code, Bytes, Cycles) \
    INSTR_DFX(Kind, Family, Code, Bytes, Cycles, 0)

/**
 * @brief All (500) Game Boy CPU instructions
 *
 */

// Used for the direct_opcode_table since some opcode values do not exist
#define OP_UNKOWN      INSTR_DEF(DIRECT, UNKN,         0x00, 1, 1)

// Direct (non-prefixed) opcodes
#define OP_ADC_A_A     INSTR_DEF(DIRECT, ADD_A_R8,     0x8F, 1, 1)
#define OP_ADC_A_B     INSTR_DEF(DIRECT, ADD_A_R8,     0x88, 1, 1)
#define OP_ADC_A_C     INSTR_DEF(DIRECT, ADD_A_R8,     0x89, 1, 1)
#define OP_ADC_A_D     INSTR_DEF(DIRECT, ADD_A_R8,     0x8A, 1, 1)
#define OP_ADC_A_E     INSTR_DEF(DIRECT, ADD_A_R8,     0x8B, 1, 1)
#define OP_ADC_A_H     INSTR_DEF(DIRECT, ADD_A_R8,     0x8C, 1, 1)
#define OP_ADC_A_HLR   INSTR_DEF(DIRECT, ADD_A_HLR,    0x8E, 1, 2)
#define OP_ADC_A_L     INSTR_DEF(DIRECT, ADD_A_R8,     0x8D, 1, 1)
#define OP_ADC_A_N8    INSTR_DEF(DIRECT, ADD_A_N8,     0xCE, 2, 2)
#define OP_ADD_A_A     INSTR_DEF(DIRECT, ADD_A_R8,     0x87, 1, 1)
#define OP_ADD_A_B     INSTR_DEF(DIRECT, ADD_A_R8,     0x80, 1, 1)
#define OP_ADD_A_C     INSTR_DEF(DIRECT, ADD_A_R8,     0x81, 1, 1)
#define OP_ADD_A_D     INSTR_DEF(DIRECT, ADD_A_R8,     0x82, 1, 1)
#define OP_ADD_A_E     INSTR_DEF(DIRECT, ADD_A_R8,     0x83, 1, 1)
#define OP_ADD_A_H     INSTR_DEF(DIRECT, ADD_A_R8,     0x84, 1, 1)
#define OP_ADD_A_HLR   INSTR_DEF(DIRECT, ADD_A_HLR,    0x86, 1, 2)
#define OP_ADD_A_L     INSTR_DEF(DIRECT, ADD_A_R8,     0x85, 1, 1)
#define OP_ADD_A_N8    INSTR_DEF(DIRECT, ADD_A_N8,     0xC6, 2, 2)
#define OP_ADD_HL_BC   INSTR_DEF(DIRECT, ADD_HL_R16SP, 0x09, 1, 2)
#define OP_ADD_HL_DE   INSTR_DEF(DIRECT, ADD_HL_R16SP, 0x19, 1, 2)
#define OP_ADD_HL_HL   INSTR_DEF(DIRECT, ADD_HL_R16SP, 0x29, 1, 2)
#define OP_ADD_HL_SP   INSTR_DEF(DIRECT, ADD_HL_R16SP, 0x39, 1, 2)
#define OP_ADD_SP_N    INSTR_DEF(DIRECT, LD_HLSP_S8,   0xE8, 2, 4)
#define OP_AND_A_A     INSTR_DEF(DIRECT, AND_A_R8,     0xA7, 1, 1)
#define OP_AND_A_B     INSTR_DEF(DIRECT, AND_A_R8,     0xA0, 1, 1)
#define OP_AND_A_C     INSTR_DEF(DIRECT, AND_A_R8,     0xA1, 1, 1)
#define OP_AND_A_D     INSTR_DEF(DIRECT, AND_A_R8,     0xA2, 1, 1)
#define OP_AND_A_E     INSTR_DEF(DIRECT, AND_A_R8,     0xA3, 1, 1)
#define OP_AND_A_H     INSTR_DEF(DIRECT, AND_A_R8,     0xA4, 1, 1)
#define OP_AND_A_HLR   INSTR_DEF(DIRECT, AND_A_HLR,    0xA6, 1, 2)
#define OP_AND_A_L     INSTR_DEF(DIRECT, AND_A_R8,     0xA5, 1, 1)
#define OP_AND_A_N8    INSTR_DEF(DIRECT, AND_A_N8,     0xE6, 2, 2)
#define OP_CALL_N16    INSTR_DEF(DIRECT, CALL_N16,     0xCD, 3, 6)
#define OP_CALL_C_N16  INSTR_DFX(DIRECT, CALL_CC_N16,  0xDC, 3, 3, 3)
#define OP_CALL_NC_N16 INSTR_DFX(DIRECT, CALL_CC_N16,  0xD4, 3, 3, 3)
#define OP_CALL_NZ_N16 INSTR_DFX(DIRECT, CALL_CC_N16,  0xC4, 3, 3, 3)
#define OP_CALL_Z_N16  INSTR_DFX(DIRECT, CALL_CC_N16,  0xCC, 3, 3, 3)
#define OP_CCF         INSTR_DEF(DIRECT, SCCF,         0x3F, 1, 1)
#define OP_CPL         INSTR_DEF(DIRECT, CPL,          0x2F, 1, 1)
#define OP_CP_A_A      INSTR_DEF(DIRECT, CP_A_R8,      0xBF, 1, 1)
#define OP_CP_A_B      INSTR_DEF(DIRECT, CP_A_R8,      0xB8, 1, 1)
#define OP_CP_A_C      INSTR_DEF(DIRECT, CP_A_R8,      0xB9, 1, 1)
#define OP_CP_A_D      INSTR_DEF(DIRECT, CP_A_R8,      0xBA, 1, 1)
#define OP_CP_A_E      INSTR_DEF(DIRECT, CP_A_R8,      0xBB, 1, 1)
#define OP_CP_A_H      INSTR_DEF(DIRECT, CP_A_R8,      0xBC, 1, 1)
#define OP_CP_A_HLR    INSTR_DEF(DIRECT, CP_A_HLR,     0xBE, 1, 2)
#define OP_CP_A_L      INSTR_DEF(DIRECT, CP_A_R8,      0xBD, 1, 1)
#define OP_CP_A_N8     INSTR_DEF(DIRECT, CP_A_N8,      0xFE, 2, 2)
#define OP_DAA         INSTR_DEF(DIRECT, DAA,          0x27, 1, 1)
#define OP_DEC_A       INSTR_DEF(DIRECT, DEC_R8,       0x3D, 1, 1)
#define OP_DEC_B       INSTR_DEF(DIRECT, DEC_R8,       0x05, 1, 1)
#define OP_DEC_BC      INSTR_DEF(DIRECT, DEC_R16SP,    0x0B, 1, 2)
#define OP_DEC_C       INSTR_DEF(DIRECT, DEC_R8,       0x0D, 1, 1)
#define OP_DEC_D       INSTR_DEF(DIRECT, DEC_R8,       0x15, 1, 1)
#define OP_DEC_DE      INSTR_DEF(DIRECT, DEC_R16SP,    0x1B, 1, 2)
#define OP_DEC_E       INSTR_DEF(DIRECT, DEC_R8,       0x1D, 1, 1)
#define OP_DEC_H       INSTR_DEF(DIRECT, DEC_R8,       0x25, 1, 1)
#define OP_DEC_HL      INSTR_DEF(DIRECT, DEC_R16SP,    0x2B, 1, 2)
#define OP_DEC_HLR     INSTR_DEF(DIRECT, DEC_HLR,      0x35, 1, 3)
#define OP_DEC_L       INSTR_DEF(DIRECT, DEC_R8,       0x2D, 1, 1)
#define OP_DEC_SP      INSTR_DEF(DIRECT, DEC_R16SP,    0x3B, 1, 2)
#define OP_DI          INSTR_DEF(DIRECT, EDI,          0xF3, 1, 1)
#define OP_EI          INSTR_DEF(DIRECT, EDI,          0xFB, 1, 1)
#define OP_HALT        INSTR_DEF(DIRECT, HALT,         0x76, 1, 1)
#define OP_INC_A       INSTR_DEF(DIRECT, INC_R8,       0x3C, 1, 1)
#define OP_INC_B       INSTR_DEF(DIRECT, INC_R8,       0x04, 1, 1)
#define OP_INC_BC      INSTR_DEF(DIRECT, INC_R16SP,    0x03, 1, 2)
#define OP_INC_C       INSTR_DEF(DIRECT, INC_R8,       0x0C, 1, 1)
#define OP_INC_D       INSTR_DEF(DIRECT, INC_R8,       0x14, 1, 1)
#define OP_INC_DE      INSTR_DEF(DIRECT, INC_R16SP,    0x13, 1, 2)
#define OP_INC_E       INSTR_DEF(DIRECT, INC_R8,       0x1C, 1, 1)
#define OP_INC_H       INSTR_DEF(DIRECT, INC_R8,       0x24, 1, 1)
#define OP_INC_HL      INSTR_DEF(DIRECT, INC_R16SP,    0x23, 1, 2)
#define OP_INC_HLR     INSTR_DEF(DIRECT, INC_HLR,      0x34, 1, 3)
#define OP_INC_L       INSTR_DEF(DIRECT, INC_R8,       0x2C, 1, 1)
#define OP_INC_SP      INSTR_DEF(DIRECT, INC_R16SP,    0x33, 1, 2)
#define OP_JP_HL       INSTR_DEF(DIRECT, JP_HL,        0xE9, 1, 1)
#define OP_JP_N16      INSTR_DEF(DIRECT, JP_N16,       0xC3, 3, 4)
#define OP_JP_C_N16    INSTR_DFX(DIRECT, JP_CC_N16,    0xDA, 3, 3, 1)
#define OP_JP_NC_N16   INSTR_DFX(DIRECT, JP_CC_N16,    0xD2, 3, 3, 1)
#define OP_JP_NZ_N16   INSTR_DFX(DIRECT, JP_CC_N16,    0xC2, 3, 3, 1)
#define OP_JP_Z_N16    INSTR_DFX(DIRECT, JP_CC_N16,    0xCA, 3, 3, 1)
#define OP_JR_E8       INSTR_DEF(DIRECT, JR_E8,        0x18, 2, 3)
#define OP_JR_C_E8     INSTR_DFX(DIRECT, JR_CC_E8,     0x38, 2, 2, 1)
#define OP_JR_NC_E8    INSTR_DFX(DIRECT, JR_CC_E8,     0x30, 2, 2, 1)
#define OP_JR_NZ_E8    INSTR_DFX(DIRECT, JR_CC_E8,     0x20, 2, 2, 1)
#define OP_JR_Z_E8     INSTR_DFX(DIRECT, JR_CC_E8,     0x28, 2, 2, 1)
#define OP_LD_A_A      INSTR_DEF(DIRECT, NOP,          0x7F, 1, 1)
#define OP_LD_A_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x78, 1, 1)
#define OP_LD_A_BCR    INSTR_DEF(DIRECT, LD_A_BCR,     0x0A, 1, 2)
#define OP_LD_A_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x79, 1, 1)
#define OP_LD_A_CR     INSTR_DEF(DIRECT, LD_A_CR,      0xF2, 1, 2)
#define OP_LD_A_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x7A, 1, 1)
#define OP_LD_A_DER    INSTR_DEF(DIRECT, LD_A_DER,     0x1A, 1, 2)
#define OP_LD_A_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x7B, 1, 1)
#define OP_LD_A_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x7C, 1, 1)
#define OP_LD_A_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x7E, 1, 2)
#define OP_LD_A_HLRD   INSTR_DEF(DIRECT, LD_A_HLRU,    0x3A, 1, 2)
#define OP_LD_A_HLRI   INSTR_DEF(DIRECT, LD_A_HLRU,    0x2A, 1, 2)
#define OP_LD_A_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x7D, 1, 1)
#define OP_LD_A_N16R   INSTR_DEF(DIRECT, LD_A_N16R,    0xFA, 3, 4)
#define OP_LD_A_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x3E, 2, 2)
#define OP_LD_A_N8R    INSTR_DEF(DIRECT, LD_A_N8R,     0xF0, 2, 3)
#define OP_LD_BCR_A    INSTR_DEF(DIRECT, LD_BCR_A,     0x02, 1, 2)
#define OP_LD_BC_N16   INSTR_DEF(DIRECT, LD_R16SP_N16, 0x01, 3, 3)
#define OP_LD_B_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x47, 1, 1)
#define OP_LD_B_B      INSTR_DEF(DIRECT, NOP,          0x40, 1, 1)
#define OP_LD_B_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x41, 1, 1)
#define OP_LD_B_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x42, 1, 1)
#define OP_LD_B_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x43, 1, 1)
#define OP_LD_B_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x44, 1, 1)
#define OP_LD_B_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x46, 1, 2)
#define OP_LD_B_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x45, 1, 1)
#define OP_LD_B_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x06, 2, 2)
#define OP_LD_CR_A     INSTR_DEF(DIRECT, LD_CR_A,      0xE2, 1, 2)
#define OP_LD_C_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x4F, 1, 1)
#define OP_LD_C_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x48, 1, 1)
#define OP_LD_C_C      INSTR_DEF(DIRECT, NOP,          0x49, 1, 1)
#define OP_LD_C_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x4A, 1, 1)
#define OP_LD_C_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x4B, 1, 1)
#define OP_LD_C_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x4C, 1, 1)
#define OP_LD_C_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x4E, 1, 2)
#define OP_LD_C_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x4D, 1, 1)
#define OP_LD_C_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x0E, 2, 2)
#define OP_LD_DER_A    INSTR_DEF(DIRECT, LD_DER_A,     0x12, 1, 2)
#define OP_LD_DE_N16   INSTR_DEF(DIRECT, LD_R16SP_N16, 0x11, 3, 3)
#define OP_LD_D_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x57, 1, 1)
#define OP_LD_D_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x50, 1, 1)
#define OP_LD_D_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x51, 1, 1)
#define OP_LD_D_D      INSTR_DEF(DIRECT, NOP,          0x52, 1, 1)
#define OP_LD_D_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x53, 1, 1)
#define OP_LD_D_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x54, 1, 1)
#define OP_LD_D_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x56, 1, 2)
#define OP_LD_D_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x55, 1, 1)
#define OP_LD_D_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x16, 2, 2)
#define OP_LD_E_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x5F, 1, 1)
#define OP_LD_E_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x58, 1, 1)
#define OP_LD_E_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x59, 1, 1)
#define OP_LD_E_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x5A, 1, 1)
#define OP_LD_E_E      INSTR_DEF(DIRECT, NOP,          0x5B, 1, 1)
#define OP_LD_E_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x5C, 1, 1)
#define OP_LD_E_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x5E, 1, 2)
#define OP_LD_E_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x5D, 1, 1)
#define OP_LD_E_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x1E, 2, 2)
#define OP_LD_HLRD_A   INSTR_DEF(DIRECT, LD_HLRU_A,    0x32, 1, 2)
#define OP_LD_HLRI_A   INSTR_DEF(DIRECT, LD_HLRU_A,    0x22, 1, 2)
#define OP_LD_HLR_A    INSTR_DEF(DIRECT, LD_HLR_R8,    0x77, 1, 2)
#define OP_LD_HLR_B    INSTR_DEF(DIRECT, LD_HLR_R8,    0x70, 1, 2)
#define OP_LD_HLR_C    INSTR_DEF(DIRECT, LD_HLR_R8,    0x71, 1, 2)
#define OP_LD_HLR_D    INSTR_DEF(DIRECT, LD_HLR_R8,    0x72, 1, 2)
#define OP_LD_HLR_E    INSTR_DEF(DIRECT, LD_HLR_R8,    0x73, 1, 2)
#define OP_LD_HLR_H    INSTR_DEF(DIRECT, LD_HLR_R8,    0x74, 1, 2)
#define OP_LD_HLR_L    INSTR_DEF(DIRECT, LD_HLR_R8,    0x75, 1, 2)
#define OP_LD_HLR_N8   INSTR_DEF(DIRECT, LD_HLR_N8,    0x36, 2, 3)
#define OP_LD_HL_N16   INSTR_DEF(DIRECT, LD_R16SP_N16, 0x21, 3, 3)
#define OP_LD_HL_SP_N8 INSTR_DEF(DIRECT, LD_HLSP_S8,   0xF8, 2, 3)
#define OP_LD_H_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x67, 1, 1)
#define OP_LD_H_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x60, 1, 1)
#define OP_LD_H_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x61, 1, 1)
#define OP_LD_H_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x62, 1, 1)
#define OP_LD_H_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x63, 1, 1)
#define OP_LD_H_H      INSTR_DEF(DIRECT, NOP,          0x64, 1, 1)
#define OP_LD_H_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x66, 1, 2)
#define OP_LD_H_L      INSTR_DEF(DIRECT, LD_R8_R8,     0x65, 1, 1)
#define OP_LD_H_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x26, 2, 2)
#define OP_LD_L_A      INSTR_DEF(DIRECT, LD_R8_R8,     0x6F, 1, 1)
#define OP_LD_L_B      INSTR_DEF(DIRECT, LD_R8_R8,     0x68, 1, 1)
#define OP_LD_L_C      INSTR_DEF(DIRECT, LD_R8_R8,     0x69, 1, 1)
#define OP_LD_L_D      INSTR_DEF(DIRECT, LD_R8_R8,     0x6A, 1, 1)
#define OP_LD_L_E      INSTR_DEF(DIRECT, LD_R8_R8,     0x6B, 1, 1)
#define OP_LD_L_H      INSTR_DEF(DIRECT, LD_R8_R8,     0x6C, 1, 1)
#define OP_LD_L_HLR    INSTR_DEF(DIRECT, LD_R8_HLR,    0x6E, 1, 2)
#define OP_LD_L_L      INSTR_DEF(DIRECT, NOP,          0x6D, 1, 1)
#define OP_LD_L_N8     INSTR_DEF(DIRECT, LD_R8_N8,     0x2E, 2, 2)
#define OP_LD_N16R_A   INSTR_DEF(DIRECT, LD_N16R_A,    0xEA, 3, 4)
#define OP_LD_N16R_SP  INSTR_DEF(DIRECT, LD_N16R_SP,   0x08, 3, 5)
#define OP_LD_N8R_A    INSTR_DEF(DIRECT, LD_N8R_A,     0xE0, 2, 3)
#define OP_LD_SP_HL    INSTR_DEF(DIRECT, LD_SP_HL,     0xF9, 1, 2)
#define OP_LD_SP_N16   INSTR_DEF(DIRECT, LD_R16SP_N16, 0x31, 3, 3)
#define OP_NOP         INSTR_DEF(DIRECT, NOP,          0x00, 1, 1)
#define OP_OR_A_A      INSTR_DEF(DIRECT, OR_A_R8,      0xB7, 1, 1)
#define OP_OR_A_B      INSTR_DEF(DIRECT, OR_A_R8,      0xB0, 1, 1)
#define OP_OR_A_C      INSTR_DEF(DIRECT, OR_A_R8,      0xB1, 1, 1)
#define OP_OR_A_D      INSTR_DEF(DIRECT, OR_A_R8,      0xB2, 1, 1)
#define OP_OR_A_E      INSTR_DEF(DIRECT, OR_A_R8,      0xB3, 1, 1)
#define OP_OR_A_H      INSTR_DEF(DIRECT, OR_A_R8,      0xB4, 1, 1)
#define OP_OR_A_HLR    INSTR_DEF(DIRECT, OR_A_HLR,     0xB6, 1, 2)
#define OP_OR_A_L      INSTR_DEF(DIRECT, OR_A_R8,      0xB5, 1, 1)
#define OP_OR_A_N8     INSTR_DEF(DIRECT, OR_A_N8,      0xF6, 2, 2)
#define OP_POP_AF      INSTR_DEF(DIRECT, POP_R16,      0xF1, 1, 3)
#define OP_POP_BC      INSTR_DEF(DIRECT, POP_R16,      0xC1, 1, 3)
#define OP_POP_DE      INSTR_DEF(DIRECT, POP_R16,      0xD1, 1, 3)
#define OP_POP_HL      INSTR_DEF(DIRECT, POP_R16,      0xE1, 1, 3)
#define OP_PUSH_AF     INSTR_DEF(DIRECT, PUSH_R16,     0xF5, 1, 4)
#define OP_PUSH_BC     INSTR_DEF(DIRECT, PUSH_R16,     0xC5, 1, 4)
#define OP_PUSH_DE     INSTR_DEF(DIRECT, PUSH_R16,     0xD5, 1, 4)
#define OP_PUSH_HL     INSTR_DEF(DIRECT, PUSH_R16,     0xE5, 1, 4)
#define OP_RET         INSTR_DEF(DIRECT, RET,          0xC9, 1, 4)
#define OP_RETI        INSTR_DEF(DIRECT, RETI,         0xD9, 1, 4)
#define OP_RET_C       INSTR_DFX(DIRECT, RET_CC,       0xD8, 1, 2, 3)
#define OP_RET_NC      INSTR_DFX(DIRECT, RET_CC,       0xD0, 1, 2, 3)
#define OP_RET_NZ      INSTR_DFX(DIRECT, RET_CC,       0xC0, 1, 2, 3)
#define OP_RET_Z       INSTR_DFX(DIRECT, RET_CC,       0xC8, 1, 2, 3)
#define OP_RLA         INSTR_DEF(DIRECT, ROTA,         0x17, 1, 1)
#define OP_RLCA        INSTR_DEF(DIRECT, ROTCA,        0x07, 1, 1)
#define OP_RRA         INSTR_DEF(DIRECT, ROTA,         0x1F, 1, 1)
#define OP_RRCA        INSTR_DEF(DIRECT, ROTCA,        0x0F, 1, 1)
#define OP_RST_0       INSTR_DEF(DIRECT, RST_U3,       0xC7, 1, 4)
#define OP_RST_1       INSTR_DEF(DIRECT, RST_U3,       0xCF, 1, 4)
#define OP_RST_2       INSTR_DEF(DIRECT, RST_U3,       0xD7, 1, 4)
#define OP_RST_3       INSTR_DEF(DIRECT, RST_U3,       0xDF, 1, 4)
#define OP_RST_4       INSTR_DEF(DIRECT, RST_U3,       0xE7, 1, 4)
#define OP_RST_5       INSTR_DEF(DIRECT, RST_U3,       0xEF, 1, 4)
#define OP_RST_6       INSTR_DEF(DIRECT, RST_U3,       0xF7, 1, 4)
#define OP_RST_7       INSTR_DEF(DIRECT, RST_U3,       0xFF, 1, 4)
#define OP_SBC_A_A     INSTR_DEF(DIRECT, SUB_A_R8,     0x9F, 1, 1)
#define OP_SBC_A_B     INSTR_DEF(DIRECT, SUB_A_R8,     0x98, 1, 1)
#define OP_SBC_A_C     INSTR_DEF(DIRECT, SUB_A_R8,     0x99, 1, 1)
#define OP_SBC_A_D     INSTR_DEF(DIRECT, SUB_A_R8,     0x9A, 1, 1)
#define OP_SBC_A_E     INSTR_DEF(DIRECT, SUB_A_R8,     0x9B, 1, 1)
#define OP_SBC_A_H     INSTR_DEF(DIRECT, SUB_A_R8,     0x9C, 1, 1)
#define OP_SBC_A_HLR   INSTR_DEF(DIRECT, SUB_A_HLR,    0x9E, 1, 2)
#define OP_SBC_A_L     INSTR_DEF(DIRECT, SUB_A_R8,     0x9D, 1, 1)
#define OP_SBC_A_N8    INSTR_DEF(DIRECT, SUB_A_N8,     0xDE, 2, 2)
#define OP_SCF         INSTR_DEF(DIRECT, SCCF,         0x37, 1, 1)
#define OP_STOP        INSTR_DEF(DIRECT, STOP,         0x10, 1, 0)
#define OP_SUB_A_A     INSTR_DEF(DIRECT, SUB_A_R8,     0x97, 1, 1)
#define OP_SUB_A_B     INSTR_DEF(DIRECT, SUB_A_R8,     0x90, 1, 1)
#define OP_SUB_A_C     INSTR_DEF(DIRECT, SUB_A_R8,     0x91, 1, 1)
#define OP_SUB_A_D     INSTR_DEF(DIRECT, SUB_A_R8,     0x92, 1, 1)
#define OP_SUB_A_E     INSTR_DEF(DIRECT, SUB_A_R8,     0x93, 1, 1)
#define OP_SUB_A_H     INSTR_DEF(DIRECT, SUB_A_R8,     0x94, 1, 1)
#define OP_SUB_A_HLR   INSTR_DEF(DIRECT, SUB_A_HLR,    0x96, 1, 2)
#define OP_SUB_A_L     INSTR_DEF(DIRECT, SUB_A_R8,     0x95, 1, 1)
#define OP_SUB_A_N8    INSTR_DEF(DIRECT, SUB_A_N8,     0xD6, 2, 2)
#define OP_XOR_A_A     INSTR_DEF(DIRECT, XOR_A_R8,     0xAF, 1, 1)
#define OP_XOR_A_B     INSTR_DEF(DIRECT, XOR_A_R8,     0xA8, 1, 1)
#define OP_XOR_A_C     INSTR_DEF(DIRECT, XOR_A_R8,     0xA9, 1, 1)
#define OP_XOR_A_D     INSTR_DEF(DIRECT, XOR_A_R8,     0xAA, 1, 1)
#define OP_XOR_A_E     INSTR_DEF(DIRECT, XOR_A_R8,     0xAB, 1, 1)
#define OP_XOR_A_H     INSTR_DEF(DIRECT, XOR_A_R8,     0xAC, 1, 1)
#define OP_XOR_A_HLR   INSTR_DEF(DIRECT, XOR_A_HLR,    0xAE, 1, 2)
#define OP_XOR_A_L     INSTR_DEF(DIRECT, XOR_A_R8,     0xAD, 1, 1)
#define OP_XOR_A_N8    INSTR_DEF(DIRECT, XOR_A_N8,     0xEE, 2, 2)

// Prefixed opcodes
#define OP_BIT_0_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x47, 2, 2)
#define OP_BIT_0_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x40, 2, 2)
#define OP_BIT_0_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x41, 2, 2)
#define OP_BIT_0_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x42, 2, 2)
#define OP_BIT_0_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x43, 2, 2)
#define OP_BIT_0_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x44, 2, 2)
#define OP_BIT_0_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x46, 2, 3)
#define OP_BIT_0_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x45, 2, 2)
#define OP_BIT_1_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x4F, 2, 2)
#define OP_BIT_1_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x48, 2, 2)
#define OP_BIT_1_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x49, 2, 2)
#define OP_BIT_1_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x4A, 2, 2)
#define OP_BIT_1_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x4B, 2, 2)
#define OP_BIT_1_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x4C, 2, 2)
#define OP_BIT_1_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x4E, 2, 3)
#define OP_BIT_1_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x4D, 2, 2)
#define OP_BIT_2_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x57, 2, 2)
#define OP_BIT_2_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x50, 2, 2)
#define OP_BIT_2_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x51, 2, 2)
#define OP_BIT_2_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x52, 2, 2)
#define OP_BIT_2_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x53, 2, 2)
#define OP_BIT_2_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x54, 2, 2)
#define OP_BIT_2_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x56, 2, 3)
#define OP_BIT_2_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x55, 2, 2)
#define OP_BIT_3_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x5F, 2, 2)
#define OP_BIT_3_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x58, 2, 2)
#define OP_BIT_3_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x59, 2, 2)
#define OP_BIT_3_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x5A, 2, 2)
#define OP_BIT_3_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x5B, 2, 2)
#define OP_BIT_3_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x5C, 2, 2)
#define OP_BIT_3_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x5E, 2, 3)
#define OP_BIT_3_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x5D, 2, 2)
#define OP_BIT_4_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x67, 2, 2)
#define OP_BIT_4_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x60, 2, 2)
#define OP_BIT_4_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x61, 2, 2)
#define OP_BIT_4_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x62, 2, 2)
#define OP_BIT_4_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x63, 2, 2)
#define OP_BIT_4_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x64, 2, 2)
#define OP_BIT_4_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x66, 2, 3)
#define OP_BIT_4_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x65, 2, 2)
#define OP_BIT_5_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x6F, 2, 2)
#define OP_BIT_5_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x68, 2, 2)
#define OP_BIT_5_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x69, 2, 2)
#define OP_BIT_5_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x6A, 2, 2)
#define OP_BIT_5_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x6B, 2, 2)
#define OP_BIT_5_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x6C, 2, 2)
#define OP_BIT_5_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x6E, 2, 3)
#define OP_BIT_5_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x6D, 2, 2)
#define OP_BIT_6_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x77, 2, 2)
#define OP_BIT_6_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x70, 2, 2)
#define OP_BIT_6_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x71, 2, 2)
#define OP_BIT_6_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x72, 2, 2)
#define OP_BIT_6_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x73, 2, 2)
#define OP_BIT_6_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x74, 2, 2)
#define OP_BIT_6_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x76, 2, 3)
#define OP_BIT_6_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x75, 2, 2)
#define OP_BIT_7_A   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x7F, 2, 2)
#define OP_BIT_7_B   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x78, 2, 2)
#define OP_BIT_7_C   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x79, 2, 2)
#define OP_BIT_7_D   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x7A, 2, 2)
#define OP_BIT_7_E   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x7B, 2, 2)
#define OP_BIT_7_H   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x7C, 2, 2)
#define OP_BIT_7_HLR INSTR_DEF(PREFIXED, BIT_U3_HLR, 0x7E, 2, 3)
#define OP_BIT_7_L   INSTR_DEF(PREFIXED, BIT_U3_R8,  0x7D, 2, 2)
#define OP_RES_0_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x87, 2, 2)
#define OP_RES_0_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x80, 2, 2)
#define OP_RES_0_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x81, 2, 2)
#define OP_RES_0_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x82, 2, 2)
#define OP_RES_0_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x83, 2, 2)
#define OP_RES_0_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x84, 2, 2)
#define OP_RES_0_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0x86, 2, 4)
#define OP_RES_0_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x85, 2, 2)
#define OP_RES_1_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x8F, 2, 2)
#define OP_RES_1_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x88, 2, 2)
#define OP_RES_1_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x89, 2, 2)
#define OP_RES_1_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x8A, 2, 2)
#define OP_RES_1_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x8B, 2, 2)
#define OP_RES_1_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x8C, 2, 2)
#define OP_RES_1_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0x8E, 2, 4)
#define OP_RES_1_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x8D, 2, 2)
#define OP_RES_2_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x97, 2, 2)
#define OP_RES_2_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x90, 2, 2)
#define OP_RES_2_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x91, 2, 2)
#define OP_RES_2_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x92, 2, 2)
#define OP_RES_2_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x93, 2, 2)
#define OP_RES_2_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x94, 2, 2)
#define OP_RES_2_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0x96, 2, 4)
#define OP_RES_2_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x95, 2, 2)
#define OP_RES_3_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x9F, 2, 2)
#define OP_RES_3_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x98, 2, 2)
#define OP_RES_3_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x99, 2, 2)
#define OP_RES_3_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x9A, 2, 2)
#define OP_RES_3_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x9B, 2, 2)
#define OP_RES_3_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x9C, 2, 2)
#define OP_RES_3_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0x9E, 2, 4)
#define OP_RES_3_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0x9D, 2, 2)
#define OP_RES_4_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA7, 2, 2)
#define OP_RES_4_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA0, 2, 2)
#define OP_RES_4_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA1, 2, 2)
#define OP_RES_4_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA2, 2, 2)
#define OP_RES_4_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA3, 2, 2)
#define OP_RES_4_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA4, 2, 2)
#define OP_RES_4_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xA6, 2, 4)
#define OP_RES_4_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA5, 2, 2)
#define OP_RES_5_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xAF, 2, 2)
#define OP_RES_5_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA8, 2, 2)
#define OP_RES_5_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xA9, 2, 2)
#define OP_RES_5_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xAA, 2, 2)
#define OP_RES_5_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xAB, 2, 2)
#define OP_RES_5_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xAC, 2, 2)
#define OP_RES_5_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xAE, 2, 4)
#define OP_RES_5_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xAD, 2, 2)
#define OP_RES_6_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB7, 2, 2)
#define OP_RES_6_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB0, 2, 2)
#define OP_RES_6_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB1, 2, 2)
#define OP_RES_6_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB2, 2, 2)
#define OP_RES_6_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB3, 2, 2)
#define OP_RES_6_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB4, 2, 2)
#define OP_RES_6_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xB6, 2, 4)
#define OP_RES_6_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB5, 2, 2)
#define OP_RES_7_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xBF, 2, 2)
#define OP_RES_7_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB8, 2, 2)
#define OP_RES_7_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xB9, 2, 2)
#define OP_RES_7_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xBA, 2, 2)
#define OP_RES_7_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xBB, 2, 2)
#define OP_RES_7_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xBC, 2, 2)
#define OP_RES_7_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xBE, 2, 4)
#define OP_RES_7_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xBD, 2, 2)
#define OP_RLC_A     INSTR_DEF(PREFIXED, ROTC_R8,    0x07, 2, 2)
#define OP_RLC_B     INSTR_DEF(PREFIXED, ROTC_R8,    0x00, 2, 2)
#define OP_RLC_C     INSTR_DEF(PREFIXED, ROTC_R8,    0x01, 2, 2)
#define OP_RLC_D     INSTR_DEF(PREFIXED, ROTC_R8,    0x02, 2, 2)
#define OP_RLC_E     INSTR_DEF(PREFIXED, ROTC_R8,    0x03, 2, 2)
#define OP_RLC_H     INSTR_DEF(PREFIXED, ROTC_R8,    0x04, 2, 2)
#define OP_RLC_HLR   INSTR_DEF(PREFIXED, ROTC_HLR,   0x06, 2, 4)
#define OP_RLC_L     INSTR_DEF(PREFIXED, ROTC_R8,    0x05, 2, 2)
#define OP_RL_A      INSTR_DEF(PREFIXED, ROT_R8,     0x17, 2, 2)
#define OP_RL_B      INSTR_DEF(PREFIXED, ROT_R8,     0x10, 2, 2)
#define OP_RL_C      INSTR_DEF(PREFIXED, ROT_R8,     0x11, 2, 2)
#define OP_RL_D      INSTR_DEF(PREFIXED, ROT_R8,     0x12, 2, 2)
#define OP_RL_E      INSTR_DEF(PREFIXED, ROT_R8,     0x13, 2, 2)
#define OP_RL_H      INSTR_DEF(PREFIXED, ROT_R8,     0x14, 2, 2)
#define OP_RL_HLR    INSTR_DEF(PREFIXED, ROT_HLR,    0x16, 2, 4)
#define OP_RL_L      INSTR_DEF(PREFIXED, ROT_R8,     0x15, 2, 2)
#define OP_RRC_A     INSTR_DEF(PREFIXED, ROTC_R8,    0x0F, 2, 2)
#define OP_RRC_B     INSTR_DEF(PREFIXED, ROTC_R8,    0x08, 2, 2)
#define OP_RRC_C     INSTR_DEF(PREFIXED, ROTC_R8,    0x09, 2, 2)
#define OP_RRC_D     INSTR_DEF(PREFIXED, ROTC_R8,    0x0A, 2, 2)
#define OP_RRC_E     INSTR_DEF(PREFIXED, ROTC_R8,    0x0B, 2, 2)
#define OP_RRC_H     INSTR_DEF(PREFIXED, ROTC_R8,    0x0C, 2, 2)
#define OP_RRC_HLR   INSTR_DEF(PREFIXED, ROTC_HLR,   0x0E, 2, 4)
#define OP_RRC_L     INSTR_DEF(PREFIXED, ROTC_R8,    0x0D, 2, 2)
#define OP_RR_A      INSTR_DEF(PREFIXED, ROT_R8,     0x1F, 2, 2)
#define OP_RR_B      INSTR_DEF(PREFIXED, ROT_R8,     0x18, 2, 2)
#define OP_RR_C      INSTR_DEF(PREFIXED, ROT_R8,     0x19, 2, 2)
#define OP_RR_D      INSTR_DEF(PREFIXED, ROT_R8,     0x1A, 2, 2)
#define OP_RR_E      INSTR_DEF(PREFIXED, ROT_R8,     0x1B, 2, 2)
#define OP_RR_H      INSTR_DEF(PREFIXED, ROT_R8,     0x1C, 2, 2)
#define OP_RR_HLR    INSTR_DEF(PREFIXED, ROT_HLR,    0x1E, 2, 4)
#define OP_RR_L      INSTR_DEF(PREFIXED, ROT_R8,     0x1D, 2, 2)
#define OP_SET_0_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC7, 2, 2)
#define OP_SET_0_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC0, 2, 2)
#define OP_SET_0_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC1, 2, 2)
#define OP_SET_0_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC2, 2, 2)
#define OP_SET_0_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC3, 2, 2)
#define OP_SET_0_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC4, 2, 2)
#define OP_SET_0_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xC6, 2, 4)
#define OP_SET_0_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC5, 2, 2)
#define OP_SET_1_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xCF, 2, 2)
#define OP_SET_1_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC8, 2, 2)
#define OP_SET_1_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xC9, 2, 2)
#define OP_SET_1_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xCA, 2, 2)
#define OP_SET_1_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xCB, 2, 2)
#define OP_SET_1_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xCC, 2, 2)
#define OP_SET_1_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xCE, 2, 4)
#define OP_SET_1_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xCD, 2, 2)
#define OP_SET_2_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD7, 2, 2)
#define OP_SET_2_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD0, 2, 2)
#define OP_SET_2_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD1, 2, 2)
#define OP_SET_2_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD2, 2, 2)
#define OP_SET_2_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD3, 2, 2)
#define OP_SET_2_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD4, 2, 2)
#define OP_SET_2_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xD6, 2, 4)
#define OP_SET_2_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD5, 2, 2)
#define OP_SET_3_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xDF, 2, 2)
#define OP_SET_3_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD8, 2, 2)
#define OP_SET_3_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xD9, 2, 2)
#define OP_SET_3_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xDA, 2, 2)
#define OP_SET_3_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xDB, 2, 2)
#define OP_SET_3_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xDC, 2, 2)
#define OP_SET_3_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xDE, 2, 4)
#define OP_SET_3_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xDD, 2, 2)
#define OP_SET_4_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE7, 2, 2)
#define OP_SET_4_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE0, 2, 2)
#define OP_SET_4_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE1, 2, 2)
#define OP_SET_4_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE2, 2, 2)
#define OP_SET_4_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE3, 2, 2)
#define OP_SET_4_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE4, 2, 2)
#define OP_SET_4_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xE6, 2, 4)
#define OP_SET_4_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE5, 2, 2)
#define OP_SET_5_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xEF, 2, 2)
#define OP_SET_5_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE8, 2, 2)
#define OP_SET_5_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xE9, 2, 2)
#define OP_SET_5_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xEA, 2, 2)
#define OP_SET_5_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xEB, 2, 2)
#define OP_SET_5_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xEC, 2, 2)
#define OP_SET_5_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xEE, 2, 4)
#define OP_SET_5_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xED, 2, 2)
#define OP_SET_6_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF7, 2, 2)
#define OP_SET_6_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF0, 2, 2)
#define OP_SET_6_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF1, 2, 2)
#define OP_SET_6_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF2, 2, 2)
#define OP_SET_6_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF3, 2, 2)
#define OP_SET_6_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF4, 2, 2)
#define OP_SET_6_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xF6, 2, 4)
#define OP_SET_6_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF5, 2, 2)
#define OP_SET_7_A   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xFF, 2, 2)
#define OP_SET_7_B   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF8, 2, 2)
#define OP_SET_7_C   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xF9, 2, 2)
#define OP_SET_7_D   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xFA, 2, 2)
#define OP_SET_7_E   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xFB, 2, 2)
#define OP_SET_7_H   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xFC, 2, 2)
#define OP_SET_7_HLR INSTR_DEF(PREFIXED, CHG_U3_HLR, 0xFE, 2, 4)
#define OP_SET_7_L   INSTR_DEF(PREFIXED, CHG_U3_R8,  0xFD, 2, 2)
#define OP_SLA_A     INSTR_DEF(PREFIXED, SLA_R8,     0x27, 2, 2)
#define OP_SLA_B     INSTR_DEF(PREFIXED, SLA_R8,     0x20, 2, 2)
#define OP_SLA_C     INSTR_DEF(PREFIXED, SLA_R8,     0x21, 2, 2)
#define OP_SLA_D     INSTR_DEF(PREFIXED, SLA_R8,     0x22, 2, 2)
#define OP_SLA_E     INSTR_DEF(PREFIXED, SLA_R8,     0x23, 2, 2)
#define OP_SLA_H     INSTR_DEF(PREFIXED, SLA_R8,     0x24, 2, 2)
#define OP_SLA_HLR   INSTR_DEF(PREFIXED, SLA_HLR,    0x26, 2, 4)
#define OP_SLA_L     INSTR_DEF(PREFIXED, SLA_R8,     0x25, 2, 2)
#define OP_SRA_A     INSTR_DEF(PREFIXED, SRA_R8,     0x2F, 2, 2)
#define OP_SRA_B     INSTR_DEF(PREFIXED, SRA_R8,     0x28, 2, 2)
#define OP_SRA_C     INSTR_DEF(PREFIXED, SRA_R8,     0x29, 2, 2)
#define OP_SRA_D     INSTR_DEF(PREFIXED, SRA_R8,     0x2A, 2, 2)
#define OP_SRA_E     INSTR_DEF(PREFIXED, SRA_R8,     0x2B, 2, 2)
#define OP_SRA_H     INSTR_DEF(PREFIXED, SRA_R8,     0x2C, 2, 2)
#define OP_SRA_HLR   INSTR_DEF(PREFIXED, SRA_HLR,    0x2E, 2, 4)
#define OP_SRA_L     INSTR_DEF(PREFIXED, SRA_R8,     0x2D, 2, 2)
#define OP_SRL_A     INSTR_DEF(PREFIXED, SRL_R8,     0x3F, 2, 2)
#define OP_SRL_B     INSTR_DEF(PREFIXED, SRL_R8,     0x38, 2, 2)
#define OP_SRL_C     INSTR_DEF(PREFIXED, SRL_R8,     0x39, 2, 2)
#define OP_SRL_D     INSTR_DEF(PREFIXED, SRL_R8,     0x3A, 2, 2)
#define OP_SRL_E     INSTR_DEF(PREFIXED, SRL_R8,     0x3B, 2, 2)
#define OP_SRL_H     INSTR_DEF(PREFIXED, SRL_R8,     0x3C, 2, 2)
#define OP_SRL_HLR   INSTR_DEF(PREFIXED, SRL_HLR,    0x3E, 2, 4)
#define OP_SRL_L     INSTR_DEF(PREFIXED, SRL_R8,     0x3D, 2, 2)
#define OP_SWAP_A    INSTR_DEF(PREFIXED, SWAP_R8,    0x37, 2, 2)
#define OP_SWAP_B    INSTR_DEF(PREFIXED, SWAP_R8,    0x30, 2, 2)
#define OP_SWAP_C    INSTR_DEF(PREFIXED, SWAP_R8,    0x31, 2, 2)
#define OP_SWAP_D    INSTR_DEF(PREFIXED, SWAP_R8,    0x32, 2, 2)
#define OP_SWAP_E    INSTR_DEF(PREFIXED, SWAP_R8,    0x33, 2, 2)
#define OP_SWAP_H    INSTR_DEF(PREFIXED, SWAP_R8,    0x34, 2, 2)
#define OP_SWAP_HLR  INSTR_DEF(PREFIXED, SWAP_HLR,   0x36, 2, 4)
#define OP_SWAP_L    INSTR_DEF(PREFIXED, SWAP_R8,    0x35, 2, 2)


// ======================================================================
/**
 * @brief Two arrays mapping opcodes to instruction: one for direct instructions
 *        and another for prefixed instructions.
 *        Content is such that tab[i].opcode == i
 *
 */
#ifndef EPFL_PPS_GBEMUL_OPCODE_C
extern
#endif
const instruction_t instruction_direct[256], instruction_prefixed[256];

// ======================================================================
/**
* @brief Macro to extract SR Bit from OPCODE
*/
#define OPCODE_SR_BIT_IDX 6
#define extract_sr_bit(op) \
    bit_get(op, OPCODE_SR_BIT_IDX)

// ======================================================================
/**
* @brief Macro to extract RotDir Bit from OPCODE
*/
#define OPCODE_ROT_DIR_IDX 3
#define extract_rot_dir(op) \
    (bit_get(op, OPCODE_ROT_DIR_IDX) ? RIGHT : LEFT)

// ======================================================================
/**
* @brief Macro to extract SCCF Bit from OPCODE
*/
#define OPCODE_SCCF_IDX 3
#define extract_sccf(op) \
    bit_get(op, OPCODE_SCCF_IDX)

// ======================================================================
/**
 * @brief Extract CC bits (flag condition) from OPCODE
 */
#define OPCODE_CC_IDX 3
#define OPCODE_CC_MASK 0x3
#define extract_cc(op) \
    (((op) >> OPCODE_CC_IDX) & OPCODE_CC_MASK)

// ======================================================================
/**
 * @brief Extract IME bit from OPCODE
 */
#define OPCODE_IME_IDX 3
#define extract_ime(op) \
    (bit_get(op, OPCODE_IME_IDX))

// =========================================================================
/**
 * @brief Extract HL increment from OPCODE
 */
#define OPCODE_HL_INDEX 4
#define extract_HL_increment(op) \
     (bit_get(op, OPCODE_HL_INDEX) ? -1 : 1)

#ifdef __cplusplus
}
#endif
