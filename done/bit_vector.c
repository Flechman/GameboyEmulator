#include "bit_vector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bit.h"


//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_create(size_t size, bit_t value) {
    if(size <= 0 || size > 65504) { return NULL; }

    size_t realSize = size;
    if (size%32 != 0) {
        realSize += 32 - (size%32);
    }
    
    bit_vector_t* pt = calloc(1, sizeof(bit_vector_t) + sizeof(uint32_t[realSize/32]));
    
    pt->size = size;
    for (int i = 0; i < realSize/32; i++){
        pt->content[i] = (value == 0 ? 0 : 0xFFFFFFFF);
    }
    if(size%32 != 0) {
        pt->content[realSize/32 - 1] = pt->content[realSize/32 - 1]>>(32 - (size%32));
    }
    return pt;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_cpy(const bit_vector_t* pbv) {
    if(pbv == NULL) { return NULL; }

    bit_vector_t *pt;
    size_t realSize = (pbv->size%32 != 0 ? pbv->size+32-(pbv->size%32) : pbv->size);
    pt = calloc(1, sizeof(bit_vector_t)+sizeof(uint32_t[realSize]));
    
    pt->size = pbv->size;
    memcpy((pt->content),(pbv->content), realSize);

    return pt;
}

//========================= see bit_vector.h ===========================
bit_t bit_vector_get(const bit_vector_t* pbv, size_t index) {
    if (pbv==NULL || pbv->size==0 || index >= pbv->size) { return 0; }
    
    return (pbv->content[index/32] >> index%32) & 1;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_not(bit_vector_t* pbv) {
    if (pbv == NULL){ return NULL; }

    uint32_t mask = 0xFFFFFFFF;
    size_t size_32 = ((pbv->size%32 == 0) ? pbv->size : pbv->size + (32 - pbv->size%32));
    for (int i = 0; i < size_32/32; i++){
        pbv->content[i] = pbv->content[i]^mask;
    }
    if(pbv->size%32 != 0) {
        pbv->content[pbv->size/32] &= (mask)>>(32 - pbv->size%32);
    }
    return pbv;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_and(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
    if (pbv1==NULL || pbv2==NULL || pbv1->size!=pbv2->size) { return NULL; }
    
    size_t size_32 = ((pbv1->size%32 == 0) ? pbv1->size : pbv1->size + (32 - pbv1->size%32));
    for (int i = 0; i < size_32/32; i++){
        pbv1->content[i] = pbv1->content[i] & pbv2->content[i];
    }
    return pbv1;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_or(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
    if (pbv1==NULL || pbv2==NULL || pbv1->size!=pbv2->size) { return NULL; }
    
    size_t size_32 = ((pbv1->size%32 == 0) ? pbv1->size : pbv1->size + (32 - pbv1->size%32));
    for (int i = 0; i < size_32/32; i++){
           pbv1->content[i] = pbv1->content[i] | pbv2->content[i];
    }
    return pbv1;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_xor(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
    if (pbv1==NULL || pbv2==NULL || pbv1->size!=pbv2->size) { return NULL; }
    
    size_t size_32 = ((pbv1->size%32 == 0) ? pbv1->size : pbv1->size + (32 - pbv1->size%32));
    for (int i = 0; i < size_32/32; i++){
           pbv1->content[i] = pbv1->content[i] ^ pbv2->content[i];
    }
    return pbv1;
}

//Auxiliary method used to return a positive result for modulo results
int64_t modulo(int64_t value, uint64_t modulus) {
    return ((value < 0) ? (value%modulus+modulus)%modulus : value%modulus);
}

//Auxiliary method to extract a chunk of 32 bits depending on index, the chunk contains the 'index'th bit
uint32_t extract_32_from_idx(const bit_vector_t* pbv, int64_t index, bit_t type) {
    //Parameter checks have been done at a higher level
    
    if(type == 0) {
        if(index < 0) {
            return 0;
        } else {
            if(index >= pbv->size && (pbv->size%32 == 0 || pbv->size/32 != index/32)) {
                return 0;
            } else {
                return pbv->content[index/32];
            }
        }
    } else {
        uint32_t toReturn = 0;
        int64_t startIndex = (index < 0) ? (index/32)*32-32 : (index/32)*32;
        int64_t endIndex = (index < 0) ? (index/32)*32 : (index/32)*32+32;
        int shift = 0;
        
        for(int64_t i=startIndex; i<endIndex; ++i) {
            bit_t b = (pbv->content[modulo(i, pbv->size)/32]>>(modulo(i, pbv->size)%32)) & 0x01;
            toReturn |= (b<<shift);
            ++shift;
        }
        return toReturn;
    }
}

//Auxiliary method to extend and extract a bit_vector_t depending on the extention mode specified by 'type"
// type=0 is zero_ext, type=1 is wrap_ext
bit_vector_t* bit_vector_extract(const bit_vector_t* pbv, int64_t index, size_t size, bit_t type) {
    //Parameter checks have been done at a higher level
    
    size_t size_32 = ((size%32 == 0) ? size : size + (32 - size%32));
    bit_vector_t* newVector = bit_vector_create(size, 0);
    int64_t tmpIndex = index;
    for(int i=0; i<size_32/32; ++i) {
        if(tmpIndex%32 == 0) {
            newVector->content[i] = extract_32_from_idx(pbv, tmpIndex, type)>>modulo(tmpIndex, 32);
        } else {
            newVector->content[i] = extract_32_from_idx(pbv, tmpIndex, type)>>modulo(tmpIndex, 32);
            newVector->content[i] |= extract_32_from_idx(pbv, tmpIndex+32, type)<<(32 - modulo(tmpIndex, 32));
        }
        tmpIndex += 32;
    }
    if(size%32 != 0) {
        newVector->content[size/32] &= (0xFFFFFFFF)>>(32 - size%32);
    }
    return newVector;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_extract_zero_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
    if(size == 0) { return NULL; }
    if(pbv == NULL) {
        return bit_vector_create(size, 0);
    }
    
    return bit_vector_extract(pbv, index, size, 0);
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_extract_wrap_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
    if(size == 0 || pbv == NULL) { return NULL; }
    
    return bit_vector_extract(pbv, index, size, 1);
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_shift(const bit_vector_t* pbv, int64_t shift) {
    if(pbv == NULL) { return NULL; }
    
    bit_vector_t* ret = bit_vector_extract_zero_ext(pbv, -shift, pbv->size);
    ret->size = pbv->size;
    return ret;
}

//========================= see bit_vector.h ===========================
bit_vector_t* bit_vector_join(const bit_vector_t* pbv1, const bit_vector_t* pbv2, int64_t shift) {
    if(pbv1 == NULL || pbv2 == NULL || pbv1->size != pbv2->size || shift < 0 || shift > pbv2->size) { return NULL; }
    
    uint32_t concatIndex = (shift%32 == 0) ? shift/32-1 : shift/32;
    bit_vector_t* new  = bit_vector_cpy(pbv1);
    for(uint32_t i=concatIndex+1; i<=((pbv2->size%32 == 0) ? pbv2->size/32-1 : pbv2->size/32); ++i) {
        new->content[i] = pbv2->content[i];
    }
    if(shift%32 != 0) {
        uint32_t mid = pbv1->content[concatIndex] & ((0xFFFFFFFF)>>(32 - (shift%32)));
        mid |= (pbv2->content[concatIndex] & ((0xFFFFFFFF)<<(shift%32)));
        new->content[concatIndex] = mid;
    }
    return new;
}

//========================= see bit_vector.h ===========================
int bit_vector_print(const bit_vector_t* pbv) {
    if(pbv != NULL) {
        for(int i=((pbv->size%32 != 0) ? pbv->size/32 : pbv->size/32-1); i>=0; --i) {
            for(int j=31; j>=0; --j) {
                uint32_t mask = 1<<j;
                if((pbv->content[i] & mask) == 0) {
                    printf("0");
                } else {
                    printf("1");
                }
            }
        }
        return pbv->size;
    } else {
        return 0;
    }
}

//========================= see bit_vector.h ===========================
int bit_vector_println(const char* prefix, const bit_vector_t* pbv) {
    if(prefix != NULL) {
            printf("%s", prefix);
    }
    if(pbv == NULL) {
        printf("\n");
        return 0;
    } else {
        int ret = bit_vector_print(pbv); printf("\n");
        return ret;
    }
}

//========================= see bit_vector.h ===========================
void bit_vector_free(bit_vector_t** pbv) {
    if(pbv != NULL && *pbv != NULL) {
        for(int i=0; i<((*pbv)->size%32 != 0 ? (*pbv)->size+32-((*pbv)->size%32) : (*pbv)->size)/32; ++i) {
            (*pbv)->content[i] = 0;
        }
        (*pbv)->size = 0;
        free(*pbv);
        *pbv = NULL;
    }
}
