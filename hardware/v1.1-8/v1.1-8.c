//
// mixed_mult_pli.c
// Mithril_Cryptography_API - PLI Interface
//
// Created by Andres Barbudo on 03-09-25.
//

#include "v1.1-8.h"
#include "15_mixed_mult.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * PLI registration function
 */
PLI_INT32 mixed_mult_register(void)
{
    s_vpi_systf_data tf_data;
    
    tf_data.type = vpiSysFunc;
    tf_data.sysfunctype = vpiIntFunc;
    tf_data.tfname = "$mixed_mult";
    tf_data.calltf = mixed_mult_call;
    tf_data.compiletf = mixed_mult_compile;
    tf_data.sizetf = NULL;
    tf_data.user_data = NULL;
    
    vpi_register_systf(&tf_data);
    return 0;
}

/**
 * PLI compile-time check function
 */
PLI_INT32 mixed_mult_compile(PLI_BYTE8 *user_data)
{
    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $mixed_mult requires at least 3 arguments\n");
        return PLI_ERROR;
    }
    
    /* Count arguments */
    while ((arg_handle = vpi_scan(arg_iterator)) != NULL) {
        arg_count++;
    }
    
    if (arg_count < 3 || arg_count > 4) {
        vpi_printf("ERROR: $mixed_mult requires 3 or 4 arguments: "
                  "(result, a, b [, modulus])\n");
        return PLI_ERROR;
    }
    
    return PLI_SUCCESS;
}

/**
 * Main PLI call function
 */
PLI_INT32 mixed_mult_call(PLI_BYTE8 *user_data)
{
    vpiHandle systf_handle, arg_iterator, arg_handle;
    vpiHandle result_handle, a_handle, b_handle, mod_handle = NULL;
    fmpz_t a, result, modulus;
    ulong b;
    int flint_result;
    PLI_INT32 return_value = PLI_SUCCESS;
    
    /* Initialize FLINT variables */
    fmpz_init(a);
    fmpz_init(result);
    fmpz_init(modulus);
    
    /* Get system task handle and arguments */
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $mixed_mult - no arguments found\n");
        return_value = PLI_ERROR;
        goto cleanup;
    }
    
    /* Get result handle (first argument) */
    result_handle = vpi_scan(arg_iterator);
    if (result_handle == NULL) {
        vpi_printf("ERROR: $mixed_mult - missing result argument\n");
        return_value = PLI_ERROR;
        goto cleanup;
    }
    
    /* Get 'a' handle (second argument) */
    a_handle = vpi_scan(arg_iterator);
    if (a_handle == NULL) {
        vpi_printf("ERROR: $mixed_mult - missing 'a' argument\n");
        return_value = PLI_ERROR;
        goto cleanup;
    }
    
    /* Get 'b' handle (third argument) */
    b_handle = vpi_scan(arg_iterator);
    if (b_handle == NULL) {
        vpi_printf("ERROR: $mixed_mult - missing 'b' argument\n");
        return_value = PLI_ERROR;
        goto cleanup;
    }
    
    /* Get optional modulus handle (fourth argument) */
    mod_handle = vpi_scan(arg_iterator);
    
    /* Convert Verilog values to FLINT types */
    convert_verilog_to_fmpz(a_handle, a);
    b = get_verilog_uint(b_handle);
    
    if (mod_handle != NULL) {
        convert_verilog_to_fmpz(mod_handle, modulus);
    }
    
    /* Perform the multiplication */
    if (mod_handle != NULL) {
        flint_result = fmpz_mul_ui_mod(result, a, b, modulus);
    } else {
        flint_result = fmpz_mul_ui_mod(result, a, b, NULL);
    }
    
    /* Handle overflow indication */
    if (flint_result == E_FLINT_OFL) {
        return_value = PLI_OVERFLOW;
        vpi_printf("INFO: $mixed_mult - modular reduction applied\n");
    }
    
    /* Convert result back to Verilog */
    convert_fmpz_to_verilog(result, result_handle);
    
cleanup:
    /* Clean up FLINT variables */
    fmpz_clear(a);
    fmpz_clear(result);
    fmpz_clear(modulus);
    
    /* Return status code */
    s_vpi_value value_s;
    value_s.format = vpiIntVal;
    value_s.value.integer = return_value;
    vpi_put_value(systf_handle, &value_s, NULL, vpiNoDelay);
    
    return PLI_SUCCESS;
}

/**
 * Helper function: Convert Verilog value to FLINT fmpz_t
 */
static void convert_verilog_to_fmpz(vpiHandle arg, fmpz_t result)
{
    s_vpi_value value_s;
    char *str_val;
    
    /* Try to get as string first for large numbers */
    value_s.format = vpiStringVal;
    vpi_get_value(arg, &value_s);
    
    if (value_s.value.str != NULL) {
        /* Convert string representation to fmpz */
        if (fmpz_set_str(result, value_s.value.str, 10) != 0) {
            /* Fallback to integer if string conversion fails */
            value_s.format = vpiIntVal;
            vpi_get_value(arg, &value_s);
            fmpz_set_si(result, value_s.value.integer);
        }
    } else {
        /* Get as integer */
        value_s.format = vpiIntVal;
        vpi_get_value(arg, &value_s);
        fmpz_set_si(result, value_s.value.integer);
    }
}

/**
 * Helper function: Convert FLINT fmpz_t to Verilog value
 */
static void convert_fmpz_to_verilog(fmpz_t src, vpiHandle result)
{
    s_vpi_value value_s;
    char *str_result;
    
    /* Convert fmpz to string */
    str_result = fmpz_get_str(NULL, 10, src);
    
    /* Set Verilog value */
    value_s.format = vpiStringVal;
    value_s.value.str = str_result;
    vpi_put_value(result, &value_s, NULL, vpiNoDelay);
    
    /* Free the string */
    flint_free(str_result);
}

/**
 * Helper function: Get unsigned long from Verilog handle
 */
static PLI_UINT32 get_verilog_uint(vpiHandle arg)
{
    s_vpi_value value_s;
    
    value_s.format = vpiIntVal;
    vpi_get_value(arg, &value_s);
    
    return (PLI_UINT32)value_s.value.integer;
}

/**
 * VPI registration array
 */
void (*vlog_startup_routines[])(void) = {
    mixed_mult_register,
    0
};
