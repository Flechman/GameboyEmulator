

#include "gameboy.h"
#include "util.h"  // for zero_init_var()
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h> // for isspace()
#include <inttypes.h> // for SCNx macro

// ======================================================================
static void error(const char* pgm, const char* msg)
{
    fputs("ERROR: ", stderr);
    if (msg != NULL) fputs(msg, stderr);
    fprintf(stderr, "\nusage:    %s input_file [iterations]\n", pgm);
    fprintf(stderr, "examples: %s rom.gb 1000\n", pgm);
    fprintf(stderr, "          %s game.gb\n", pgm);
}

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
    fprintf(file, "PC: " PRPAIR "\n", cpu->PC);
    fprintf(file, "SP: " PRPAIR "\n", cpu->SP);
    fprintf(file, "IME: %u, IE: " PRREG ", IF: " PRREG ", HALT: %u\n",
            cpu->IME, cpu->IE, cpu->IF, cpu->HALT);
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
int main(int argc, char* argv[])
{
    if (argc < 2) {
        error(argv[0], "please provide input_file");
        return 1;
    }

    const char* const filename = argv[1];

    gameboy_t gb;
    zero_init_var(gb);
    int err = gameboy_create(&gb, filename);
    if (err != ERR_NONE) {
        gameboy_free(&gb);
        return err;
    }

    uint64_t cycle = 1;
    if (argc > 2) {
        cycle = (uint64_t) atoll(argv[2]);
    }

    err = gameboy_run_until(&gb, cycle);
    if (err == ERR_NONE) {
        cpu_dump_to_file("dump_cpu.txt", &(gb.cpu));
        mem_dump_to_file("dump_mem.bin", gb.components);
    }

    gameboy_free(&gb);

    return err;
}
