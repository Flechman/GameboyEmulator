

#include <stdio.h>
#include <stdlib.h>
#include "cartridge.h"
#include "error.h"

int cartridge_init_from_file(component_t* cartridge, const char* filename){
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(cartridge);
    
    data_t data[BANK_ROM_SIZE];
    FILE* file = NULL;
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr,"Erreur : impossible d'ouvrir le fichier \n");
        return ERR_IO;
    } else {
        size_t read = fread(data, sizeof(data_t), BANK_ROM_SIZE, file);
        
        if(read<BANK_ROM_SIZE){
           fprintf(stderr,"Erreur : le fichier s'est terminer trop tôt: indice = %zu \n",read+1);
            fclose(file);
            return ERR_IO;
        }
        //If the Rom file is longer than BANK_ROM_SIZE we choosed to just take the first BANK_ROM_SIZE bytes and ignore the rest
        if (ferror(file)){
            fprintf(stderr,"Erreur : impossible de lire le %zu ieme byte du fichier \n",read+1);
            fclose(file);
            return ERR_IO;
        }
        fclose(file);
        if (data[CARTRIDGE_TYPE_ADDR] != 0){
            return ERR_NOT_IMPLEMENTED;
        }
        memcpy((cartridge->mem->memory), data, BANK_ROM_SIZE);
        return ERR_NONE;
    }
}

int cartridge_init(cartridge_t* ct, const char* filename){
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(ct);
    M_REQUIRE_NO_ERR(component_create(&(ct->c), BANK_ROM_SIZE));
    error_code err=cartridge_init_from_file(&(ct->c),filename);
    if (err!=ERR_NONE){
        cartridge_free(ct);
        return err;
    }
    return ERR_NONE;
}

int cartridge_plug(cartridge_t* ct, bus_t bus){
    M_REQUIRE_NON_NULL(ct);
    M_REQUIRE_NON_NULL(bus);
   return bus_forced_plug(bus, &(ct->c), BANK_ROM0_START, BANK_ROM1_END, 0);
}

void cartridge_free(cartridge_t* ct){
    if (ct != NULL){
        component_free(&(ct->c));
    }
}
