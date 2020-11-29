
#include <stdio.h>
#include "bit.h"
#include "alu.h"
#include "error.h"


flag_bit_t get_flag(flags_t flags, flag_bit_t flag) {
    if(flag != FLAG_Z && flag != FLAG_N && flag != FLAG_H && flag != FLAG_C) { return 0; }
    return (flags & flag);
}

void set_flag(flags_t* flags, flag_bit_t flag) {
    if(flags != NULL && (flag == FLAG_Z || flag == FLAG_N || flag == FLAG_H || flag == FLAG_C)) {
        *flags |= flag;
    }
}

void unset_flag(flags_t* flags, flag_bit_t flag) {
    if(flags != NULL && (flag == FLAG_Z || flag == FLAG_N || flag == FLAG_H || flag == FLAG_C)) {
        *flags &= ~flag;
    }
}

//Auxiliary method to unset all the flags
void unset_all(alu_output_t* result) {
	//We assume here that result has already been checked at a higher level
    unset_Z(&result->flags);
    unset_N(&result->flags);
    unset_H(&result->flags);
    unset_C(&result->flags);
}

int alu_add8(alu_output_t* result, uint8_t x, uint8_t y, bit_t c0) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(c0<=1, ERR_BAD_PARAMETER, "input c0 is out of bounds: %d\n", c0);
    //Initializes all flags of result to unset
    unset_all(result);
    uint8_t v1 = lsb4(x) + lsb4(y) + c0;
	uint8_t v2 = msb4(x) + msb4(y) + msb4(v1);
	result->value =  merge4(v1, v2);
    if(result->value == 0) { set_Z(&result->flags); }
    if(msb4(v1) != 0) { set_H(&result->flags); }
    if(msb4(v2) != 0) { set_C(&result->flags); }
    return ERR_NONE;
}

int alu_sub8(alu_output_t* result, uint8_t x, uint8_t y, bit_t b0) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(b0<=1, ERR_BAD_PARAMETER, "input b0 is out of bounds: %d\n", b0);
    //Initializes all flags of result to unset
    unset_all(result);
    uint8_t v1 = lsb4(x) - lsb4(y) - lsb4(b0);
    uint8_t vv1 = msb4(v1) != 0 ? UINT8_MAX : 0;
    uint8_t v2 = msb4(x) - msb4(y) + msb4(v1);
    uint8_t vv2 = msb4(x) - msb4(y) + vv1;
    result->value = merge4(v1, v2);
    if(msb4(v1) != 0) { set_H(&result->flags); }
    if((msb4(vv2)) != 0) { set_C(&result->flags); }
    if(result->value == 0) { set_Z(&result->flags); }
    set_N(&result->flags);
    return ERR_NONE;
}

int alu_add16_both(alu_output_t* result, uint16_t x, uint16_t y, rot_dir_t dir) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(dir==LEFT || dir==RIGHT, ERR_BAD_PARAMETER, "input dir (%d) is not a valid rot_dir_t\n", dir);
	//Initializes all flags of result to unset
    unset_all(result);
    alu_output_t tmpRes;
    tmpRes.flags = 0;
    tmpRes.value = 0;
    uint16_t v1 = lsb8(x) + lsb8(y);
    uint16_t v2 = msb8(x) + msb8(y) + msb8(v1);
    bit_t carry = bit_get(msb8(v1), 0);
    if(((dir == LEFT) ? alu_add8(&tmpRes, msb8(x), msb8(y), carry) : alu_add8(&tmpRes, lsb8(x), lsb8(y), 0)) != ERR_NONE) {
        return ERR_BAD_PARAMETER;
    }
    result->value = merge8(v1, v2);
    result->flags = tmpRes.flags;
    result->value == 0 ? set_Z(&result->flags) : unset_Z(&result->flags);
    return ERR_NONE;
}

int alu_add16_low(alu_output_t* result, uint16_t x, uint16_t y){
    return alu_add16_both(result, x, y, RIGHT);
}

int alu_add16_high(alu_output_t* result, uint16_t x, uint16_t y){
    return alu_add16_both(result, x, y, LEFT);
}

int alu_shift(alu_output_t* result, uint8_t x, rot_dir_t dir) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(dir==LEFT || dir==RIGHT, ERR_BAD_PARAMETER, "input dir (%d) is not a valid rot_dir_t\n", dir);
    //Initializes all flags of result to unset
    unset_all(result);
    if(dir == LEFT) {
        result->value = lsb8(x<<1);
        if(bit_get(x, UINT8_SIZE-1) != 0) { set_C(&result->flags); }
    } else {
        result->value = lsb8(x>>1);
        if(bit_get(x, 0) != 0) { set_C(&result->flags); }
    }
    if(result->value == 0) { set_Z(&result->flags); }
    return ERR_NONE;
}

int alu_shiftR_A(alu_output_t* result, uint8_t x) {
	M_REQUIRE_NON_NULL(result);
	//Initializes all flags of result to unset
    unset_all(result);
    uint8_t shifted = x>>1;
    if(bit_get(x, UINT8_SIZE-1) != 0) {
        bit_set(&shifted, UINT8_SIZE-1);
    }
    result->value = shifted;
    if(bit_get(x, 0) != 0) { set_C(&result->flags); }
    if(result->value == 0) { set_Z(&result->flags); }
    return ERR_NONE;
}

int alu_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(dir<=1, ERR_BAD_PARAMETER, "input dir (%d) is not a valid rot_dir_t\n", dir);
    //Initializes all flags of result to unset
    unset_all(result);
    if(dir==LEFT) {
        if(bit_get(x, UINT8_SIZE-1) != 0) { set_C(&result->flags); }
    } else {
        if(bit_get(x, 0) != 0) { set_C(&result->flags); }
    }
    bit_rotate(&x, dir, 1);
    result->value = x;
    if(result->value == 0) { set_Z(&result->flags); }
    return ERR_NONE;
}

int alu_carry_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir, flags_t flags) {
    M_REQUIRE_NON_NULL(result);
    M_REQUIRE(dir<=1, ERR_BAD_PARAMETER, "input dir (%d) is not a valid rot_dir_t\n", dir);
    //Initializes all flags of result to unset
    unset_all(result);
    flag_bit_t c = get_C(flags);
    if(dir==LEFT) {
        if(bit_get(x, UINT8_SIZE-1) != 0) { set_C(&result->flags); }
        x = x<<1;
        if(c != 0) {
            x = x | 1;
        }
    } else {
        if(bit_get(x, 0) != 0) { set_C(&result->flags); }
        x = x>>1;
        if(c != 0) {
            x = x | (1<<(UINT8_SIZE-1));
        }
    }
    result->value = x;
    if(result->value == 0) { set_Z(&result->flags); }
    return ERR_NONE;
}

