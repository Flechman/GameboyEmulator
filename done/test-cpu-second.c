

#include "opcode.h" // opcode_check_integrity()
#include "cpu.h"
#include "cpu-storage.h" // cpu_read_at_idx()
#include "util.h"  // for SIZE_T_FMT
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <inttypes.h> // PRIX8, etc.

const opcode_t instructions[] = {

    // suite de Fibonacci

    0x31, 0xFF, 0xFF, // LD SP, $FFFF
    0x3E, 0x0B,       // LD A, 11
    0xCD, 0x0A, 0x00, // CALL $000A
    0x76,             // HALT
    0x00,             // NOP
    0xFE, 0x02,       // CP A, 2
    0xD8,             // RET C
    0xC5,             // PUSH BC
    0x3D,             // DEC A
    0x47,             // LD B, A
    0xCD, 0x0A, 0x00, // CALL $000A
    0x4F,             // LD C, A
    0x78,             // LD A, B
    0x3D,             // DEC A
    0xCD, 0x0A, 0x00, // CALL $000A
    0x81,             // ADD A, C
    0xC1,             // POP BC
    0xC9              // RET
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
