

#include "opcode.h" // opcode_check_integrity()
#include "cpu.h"
#include "cpu-storage.h" // cpu_read_at_idx()
#include "util.h"  // for SIZE_T_FMT
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <inttypes.h> // PRIX8, etc.

const opcode_t instructions[] = {

    // Que des NOP

    0x00,       // NOP
    0x7F,       // LD A, A = NOP
    0x40,       // LD B, B = NOP
    0x49,       // LD C, C = NOP
    0x51,       // LD D, D = NOP
    0x5B,       // LD E, E = NOP
    0x64,       // LD H, H = NOP
    0x6D,       // LD L, L = NOP
    0x00,       // NOP  (to print; see line 193 below)

    // Diverses instructions

    0x33,       // INC SP       (INC_R16SP)
    0x33,       // INC SP
    0x33,       // INC SP
    0x33,       // INC SP
    0x33,       // INC SP                   ---> SP = 5
    0x23,       // INC HL       (INC_R16SP)
    0x23,       // INC HL
    0x23,       // INC HL                   ---> HL = 3
    0x00,       // NOP
    0x34,       // INC [HL]     (INC_HLR)   ---> ++bus[HL] : 0x49 --> 0x4A
    0xBD,       // CP A, L      (CP_A_R8)   ---> A < 3 --> F = Z1HC0000b = 01110000b = 0x70
    0x00,       // NOP
    0x86,       // ADD A, [HL]  (ADD_A_HLR) ---> A = 0x00 + 0x4A = 0x4A
    0x00,       // NOP
    0xCE, 0x02, // ADC A, 0x02              ---> A = 0x4A + 0x02 = 0x4C
    0x00,       // NOP
    0x85,       // ADD A, L     (ADD_A_R8)  ---> A = 0x4C + 0x03 = 0x4F
    0x00,       // NOP
    0x3C,       // INC A        (INC_R8)    ---> ++A --> 0x50
    0x00,       // NOP
    0x33,       // INC SP       (INC_R16SP)    ---> ++SP : 6
    0x39,       // ADD HL, SP   (ADD_HL_R16SP) --->  HL = 0x03 + 0x06 = 0x09
    0xBD,       // CP A, L      (CP_A_R8)      ---> A > 9  --> F = Z1HC0000b = 01100000b = 0x60
    0x00,       // NOP
    0xCB, 0xC7, // SET 0, A     (CHG_U3_R8)    ---> A = A & 0x01 = 0x51
    0x00,       // NOP
    0xCB, 0x17, // RL A         (ROT_R8)       ---> A = 0x51 << 1 = 0xA2
    0x00,       // NOP
    0xCB, 0x57, // BIT 2, A     (BIT_U3_R8)    ---> BIT(A,2) == 0 --> Z = 1 --> F = 10100000b = 0xA0
    0x00,       // NOP
    0xCB, 0x27, // SLA A        (SLA_R8)       ---> A = 0xA2 << 1 = 0x44 and C set to 1 --> F = 0x10
    0x00,       // NOP

    // 5 boucles « de Fibonacci »

    0x06, 0x00, // LD B, 0
    0x3E, 0x01, // LD A, 1
    0x0E, 0x0A, // LD C, 10
    0x57,       // LD D, A
    0x80,       // ADD A, B
    0x42,       // LD B, D
    0x00,       // NOP
    0x57,       // LD D, A
    0x80,       // ADD A, B
    0x42,       // LD B, D
    0x00,       // NOP
    0x57,       // LD D, A
    0x80,       // ADD A, B
    0x42,       // LD B, D
    0x00,       // NOP
    0x57,       // LD D, A
    0x80,       // ADD A, B
    0x42,       // LD B, D
    0x00,       // NOP
    0x57,       // LD D, A
    0x80,       // ADD A, B
    0x42,       // LD B, D
    0x00,       // NOP
};

// ======================================================================
int mem_dump_to_file(const char* filename, component_t* c)
{
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c->mem);
    M_REQUIRE_NON_NULL(c->mem->memory);
    M_REQUIRE_NON_NULL(filename);

    FILE* file = fopen(filename, "wb");
    M_EXIT_IF(file == NULL, ERR_IO,
              "cannot open file \"%s\" for writing (binary mode)\n", filename);

    const size_t check = fwrite(c->mem->memory, 1, c->mem->size, file);

    fclose(file);

    if (check != c->mem->size) {
        M_EXIT_ERR(ERR_IO,
                   "was unable to dump " SIZE_T_FMT " bytes in file \"%s\"; wrote only " SIZE_T_FMT " bytes.\n",
                   c->mem->size, filename, check);
    }

    return ERR_NONE;
}

// ======================================================================
#define PRREG  "0x%02" PRIX8
#define PRPAIR "0x%04" PRIX16
void cpu_dump(FILE* file, cpu_t* cpu)
{
    fprintf(file, "REGS: " PRREG ", " PRREG ", " PRREG ", " PRREG ", " PRREG ", " PRREG ", " PRREG ", " PRREG "\n",
            cpu->A, cpu->B, cpu->C, cpu->D, cpu->E, cpu->F, cpu->H, cpu->L);
    fprintf(file, "REGPAIRS: " PRPAIR ", " PRPAIR ", " PRPAIR ", " PRPAIR "\n",
            cpu->AF, cpu->BC,  cpu->DE, cpu->HL);
    fprintf(file, "PC: %" PRIu16 "\n", cpu->PC);
    fprintf(file, "SP: %" PRIu16 "\n", cpu->SP);
}

// ======================================================================
int cpu_dump_to_file(const char* filename, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(filename);

    FILE* file = fopen(filename, "w");
    M_EXIT_IF(file == NULL, ERR_IO,
              "cannot open file \"%s\" for writing (binary mode)\n", filename);

    cpu_dump(file, cpu);
    fclose(file);

    return ERR_NONE;
}

// ======================================================================
int main(int argc, char *argv[])
{
    if (!opcode_check_integrity()) {
        fputs("incoherent code. fix opcode.[ch]\n", stderr);
        return 1;
    }

    uint64_t cycle = 1;

    if (argc > 1) {
        cycle = (uint64_t) atol(argv[1]);
    }

    bus_t bus = {0};
    component_t c = {NULL, 0, 0};

#define TEST_CARTRIDGE_SIZE HIGH_RAM_START
    M_EXIT_IF_ERR(component_create(&c, TEST_CARTRIDGE_SIZE));
    assert(sizeof(instructions) <= TEST_CARTRIDGE_SIZE);
    memcpy(c.mem->memory, instructions, sizeof(instructions));

    cpu_t cpu;
    zero_init_var(cpu);
    M_EXIT_IF_ERR(cpu_init(&cpu));
    M_EXIT_IF_ERR_DO_SOMETHING(bus_plug(bus, &c, 0, (addr_t)(TEST_CARTRIDGE_SIZE - 1)),
                               component_free(&c));
    M_EXIT_IF_ERR_DO_SOMETHING(cpu_plug(&cpu, &bus),
                               component_free(&c));

    error_code err = ERR_NONE;
    printf("Starting running CPU for %lu cycles\n", cycle);
    do {
        fputs(cpu.idle_time >= 1 ? "Waiting to execute" : "Executing", stdout);
        data_t code = cpu_read_at_idx(&cpu, cpu.PC);
        uint8_t cycles = 0;
        if (code == PREFIXED) {
            fputs(" (prefixed)", stdout);
            code = cpu_read_at_idx(&cpu, (addr_t) (cpu.PC + 1));
            cycles = (uint8_t) (instruction_prefixed[code].cycles + instruction_prefixed[code].xtra_cycles);
        } else {
            cycles = (uint8_t) (instruction_direct[code].cycles + instruction_direct[code].xtra_cycles);
        }
        printf(": 0x%02" PRIX8 " (%" PRIu8 " cycle(s))\n", code, cycles);
        if ((cpu.idle_time == 0) && (code == 0)) { // On (real) NOP: print CPU status
            cpu_dump(stdout, &cpu);
            if (err != ERR_NONE) {
                printf(" ==> But got ERROR \"%s\"\n", ERR_MESSAGES[err]);
            }
        }

        err = cpu_cycle(&cpu);
        cycle--;
    } while (err == ERR_NONE && cycle > 0);

    if (err == ERR_NONE) {
        (void)cpu_dump_to_file("dump_cpu.txt", &cpu);
        (void)mem_dump_to_file("dump_mem.bin", &c);
    }

    cpu_free(&cpu);
    component_free(&c);

    return 0;
}
