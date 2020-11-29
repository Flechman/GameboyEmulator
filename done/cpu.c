

#include "error.h"
#include "opcode.h"
#include "cpu.h"
#include "cpu-alu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "util.h"

#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf

int cc_is_true(cpu_t* cpu, opcode_t op);

// ======================================================================
int cpu_init(cpu_t* cpu) {
    M_REQUIRE_NON_NULL(cpu);
    cpu->idle_time = 0;
    alu_output_t aluOut = { 0, 0 };
    cpu->alu = aluOut;
    M_REQUIRE_NO_ERR(component_create(&cpu->high_ram, HIGH_RAM_SIZE));
    cpu_AF_set(cpu, 0);
    cpu_BC_set(cpu, 0);
    cpu_DE_set(cpu, 0);
    cpu_HL_set(cpu, 0);
    cpu_reg_pair_SP_set(cpu, REG_AF_CODE, 0);
    cpu->PC = 0;
    cpu->IE = 0;
    cpu->IF = 0;
    cpu->IME = 0;
    cpu->HALT = 0;
    cpu->write_listener = 0;
    return ERR_NONE;
}

// ======================================================================
int cpu_plug(cpu_t* cpu, bus_t* bus)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(bus);
    (*bus)[REG_IE] = &cpu->IE;
    (*bus)[REG_IF] = &cpu->IF;
    cpu->bus = bus;
    M_REQUIRE_NO_ERR(bus_plug(*cpu->bus, &cpu->high_ram, HIGH_RAM_START, HIGH_RAM_END));
    return ERR_NONE;
}

// ======================================================================
void cpu_free(cpu_t* cpu)
{
    if(cpu != NULL) {
        bus_unplug(*(cpu->bus), &(cpu->high_ram));
        component_free(&cpu->high_ram);
        cpu->bus = NULL;
    }
}

//=========================================================================
/**
 * @brief Executes an instruction
 * @param lu instruction
 * @param cpu the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
static int cpu_dispatch(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(lu);
    M_REQUIRE_NON_NULL(cpu);
    int xtra = 0;

    cpu->alu.value = 0;
    cpu->alu.flags = 0;

    switch (lu->family) {

    // ALU
    case ADD_A_HLR:
    case ADD_A_N8:
    case ADD_A_R8:
    case INC_HLR:
    case INC_R8:
    case ADD_HL_R16SP:
    case INC_R16SP:
    case SUB_A_HLR:
    case SUB_A_N8:
    case SUB_A_R8:
    case DEC_HLR:
    case DEC_R8:
    case DEC_R16SP:
    case AND_A_HLR:
    case AND_A_N8:
    case AND_A_R8:
    case OR_A_HLR:
    case OR_A_N8:
    case OR_A_R8:
    case XOR_A_HLR:
    case XOR_A_N8:
    case XOR_A_R8:
    case CPL:
    case CP_A_HLR:
    case CP_A_N8:
    case CP_A_R8:
    case SLA_HLR:
    case SLA_R8:
    case SRA_HLR:
    case SRA_R8:
    case SRL_HLR:
    case SRL_R8:
    case ROTCA:
    case ROTA:
    case ROTC_HLR:
    case ROT_HLR:
    case ROTC_R8:
    case ROT_R8:
    case SWAP_HLR:
    case SWAP_R8:
    case BIT_U3_HLR:
    case BIT_U3_R8:
    case CHG_U3_HLR:
    case CHG_U3_R8:
    case LD_HLSP_S8:
    case DAA:
    case SCCF:
        M_EXIT_IF_ERR(cpu_dispatch_alu(lu, cpu));
        break;

    // STORAGE
    case LD_A_BCR:
    case LD_A_CR:
    case LD_A_DER:
    case LD_A_HLRU:
    case LD_A_N16R:
    case LD_A_N8R:
    case LD_BCR_A:
    case LD_CR_A:
    case LD_DER_A:
    case LD_HLRU_A:
    case LD_HLR_N8:
    case LD_HLR_R8:
    case LD_N16R_A:
    case LD_N16R_SP:
    case LD_N8R_A:
    case LD_R16SP_N16:
    case LD_R8_HLR:
    case LD_R8_N8:
    case LD_R8_R8:
    case LD_SP_HL:
    case POP_R16:
    case PUSH_R16:
        M_EXIT_IF_ERR(cpu_dispatch_storage(lu, cpu));
        break;


    // JUMP
    case JP_CC_N16:
            if(cc_is_true(cpu, lu->opcode)) {
                cpu->PC = cpu_read_addr_after_opcode(cpu)-lu->bytes;
                xtra = lu->xtra_cycles;
            }
        break;

    case JP_HL:
            cpu->PC = cpu_reg_pair_get(cpu, REG_HL_CODE)-lu->bytes;
        break;

    case JP_N16:
            cpu->PC = cpu_read_addr_after_opcode(cpu)-lu->bytes;
        break;

    case JR_CC_E8:
            if(cc_is_true(cpu, lu->opcode)) {
                cpu->PC += (int8_t)cpu_read_data_after_opcode(cpu);
                xtra = lu->xtra_cycles;
            }
        break;

    case JR_E8:
            cpu->PC += (int8_t)cpu_read_data_after_opcode(cpu);
        break;


    // CALLS
    case CALL_CC_N16:
        if(cc_is_true(cpu, lu->opcode)) {
            M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC+lu->bytes));
            cpu->PC = cpu_read_addr_after_opcode(cpu)-lu->bytes;
            xtra = lu->xtra_cycles;
        }
        break;

    case CALL_N16:
        M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC+lu->bytes));
        cpu->PC = cpu_read_addr_after_opcode(cpu)-lu->bytes;
        break;


    // RETURN (from call)
    case RET:
        cpu->PC = cpu_SP_pop(cpu)-lu->bytes;
        break;

    case RET_CC:
        if(cc_is_true(cpu, lu->opcode)) {
            cpu->PC = cpu_SP_pop(cpu)-lu->bytes;
            xtra = lu->xtra_cycles;
        }
        break;

    case RST_U3:
        M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC+lu->bytes));
        cpu->PC = (extract_reg_at_three(lu->opcode)<<3)-lu->bytes;
        break;


    // INTERRUPT & MISC.
    case EDI:
        cpu->IME = extract_ime(lu->opcode);
        break;

    case RETI:
        cpu->IME = 1;
        cpu->PC = cpu_SP_pop(cpu)-lu->bytes;
        break;

    case HALT:
        cpu->HALT = 1;
        break;

    case STOP:
    case NOP:
        // ne rien faire
        break;

    default: {
        fprintf(stderr, "Unknown instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
    } break;

    } // switch

    cpu->idle_time = lu->cycles-1+xtra;
    cpu->PC += lu->bytes;

    return ERR_NONE;
}

// ----------------------------------------------------------------------
static int cpu_do_cycle(cpu_t* cpu)
{
    if(cpu->IME != 0) {
        int interruptIdx = extract_interrupt_index_if_enable(cpu);
        if(interruptIdx != -1) {
            cpu->IME = 0;
            bit_unset(&cpu->IF, interruptIdx);
            cpu_SP_push(cpu, cpu->PC);
            cpu->PC = INTERRUPT_ADDR + (interruptIdx<<3);
            cpu->idle_time += 5;
            return ERR_NONE;
        }
    }
    opcode_t opc = cpu_read_at_idx(cpu, cpu->PC);
    if(opc == PREFIXED){
        cpu_dispatch(&instruction_prefixed[cpu_read_data_after_opcode(cpu)], cpu);
    } else {
        cpu_dispatch(&instruction_direct[opc], cpu);
    }
    return ERR_NONE;
}

// ======================================================================
/**
 * See cpu.h
 */
int cpu_cycle(cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    cpu->write_listener = 0;
    if(cpu->idle_time != 0) {
        --(cpu->idle_time);
        return ERR_NONE;
    }
    if(cpu->HALT != 0) {
       if(extract_interrupt_index_if_enable(cpu) != -1) {
            cpu->HALT = 0;
            M_REQUIRE_NO_ERR(cpu_do_cycle(cpu));
        }
    } else {
        M_REQUIRE_NO_ERR(cpu_do_cycle(cpu));
    }
    return ERR_NONE;
}
// ======================================================================
void cpu_request_interrupt(cpu_t* cpu, interrupt_t i) 
{
    if(cpu != NULL) {
        bit_set(&cpu->IF, i);
    }
}

// ======================================================================
int extract_interrupt_index_if_enable(cpu_t* cpu) 
{    
    for (int i=0; i<INTERRUPT_COUNT; ++i) {        
        int enable = cpu->IE>>i & 1;        
        int flag = cpu->IF>>i & 1;        
        if (enable==1 && flag==1) {            
            return i;        
        }
    }
    return -1;
}
// ======================================================================
/*
* Helper method for instructions
*/
int cc_is_true(cpu_t* cpu, opcode_t op) {
    switch (extract_cc(op)) {
        case 0:
            return (bit_get(cpu->F, 7)==0 ? 1 : 0);
        case 1:
            return (bit_get(cpu->F, 7)==1 ? 1 : 0);
        case 2:
            return (bit_get(cpu->F, 4)==0 ? 1 : 0);
        case 3:
            return (bit_get(cpu->F, 4)==1 ? 1 : 0);
        default:
            return 0;
    }
}
