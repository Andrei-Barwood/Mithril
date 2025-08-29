//
// increment_pli.c
// Mithril_Cryptography_API PLI Module
//
// PLI wrapper for secure increment operations
//

#include "v1.1-5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility function to convert Verilog string to fmpz_t */
static int verilog_str_to_fmpz(fmpz_t result, const char *str) {
    if (fmpz_set_str(result, str, 10) != 0) {
        vpi_printf("ERROR: Invalid number format: %s\n", str);
        return -1;
    }
    return 0;
}

/* Utility function to convert fmpz_t to Verilog string */
static char* fmpz_to_verilog_str(const fmpz_t value) {
    char *str = fmpz_get_str(NULL, 10, value);
    return str;
}

/* fmpz_inc PLI implementation */
PLI_INT32 fmpz_inc_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    PLI_INT32 arg_type;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $fmpz_inc requires 2 arguments (result, input)\n");
        return 1;
    }
    
    while ((arg_handle = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
        arg_type = vpi_get(vpiType, arg_handle);
        if (arg_type != vpiReg && arg_type != vpiMemoryWord) {
            vpi_printf("ERROR: $fmpz_inc arguments must be registers\n");
            return 1;
        }
    }
    
    if (arg_count != 2) {
        vpi_printf("ERROR: $fmpz_inc requires exactly 2 arguments\n");
        return 1;
    }
    
    return 0;
}

PLI_INT32 fmpz_inc_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    vpiHandle result_handle, input_handle;
    s_vpi_value value;
    fmpz_t a, result;
    char *result_str;
    int status;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    result_handle = vpi_scan(arg_iter);
    input_handle = vpi_scan(arg_iter);
    
    /* Get input value */
    value.format = vpiStringVal;
    vpi_get_value(input_handle, &value);
    
    /* Initialize FLINT integers */
    fmpz_init(a);
    fmpz_init(result);
    
    /* Convert input string to fmpz_t */
    if (verilog_str_to_fmpz(a, value.value.str) != 0) {
        fmpz_clear(a);
        fmpz_clear(result);
        return 1;
    }
    
    /* Perform increment operation */
    fmpz_add_ui(result, a, 1UL);
    
    /* Convert result back to string */
    result_str = fmpz_to_verilog_str(result);
    
    /* Set result in Verilog */
    value.format = vpiStringVal;
    value.value.str = result_str;
    vpi_put_value(result_handle, &value, NULL, vpiNoDelay);
    
    /* Cleanup */
    free(result_str);
    fmpz_clear(a);
    fmpz_clear(result);
    
    return 0;
}

/* fmpz_inc_inplace PLI implementation */
PLI_INT32 fmpz_inc_inplace_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    PLI_INT32 arg_type;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $fmpz_inc_inplace requires 1 argument\n");
        return 1;
    }
    
    while ((arg_handle = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
        arg_type = vpi_get(vpiType, arg_handle);
        if (arg_type != vpiReg && arg_type != vpiMemoryWord) {
            vpi_printf("ERROR: $fmpz_inc_inplace argument must be a register\n");
            return 1;
        }
    }
    
    if (arg_count != 1) {
        vpi_printf("ERROR: $fmpz_inc_inplace requires exactly 1 argument\n");
        return 1;
    }
    
    return 0;
}

PLI_INT32 fmpz_inc_inplace_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value value;
    fmpz_t a;
    char *result_str;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    arg_handle = vpi_scan(arg_iter);
    
    /* Get current value */
    value.format = vpiStringVal;
    vpi_get_value(arg_handle, &value);
    
    /* Initialize FLINT integer */
    fmpz_init(a);
    
    /* Convert input string to fmpz_t */
    if (verilog_str_to_fmpz(a, value.value.str) != 0) {
        fmpz_clear(a);
        return 1;
    }
    
    /* Perform in-place increment */
    fmpz_add_ui(a, a, 1UL);
    
    /* Convert result back to string */
    result_str = fmpz_to_verilog_str(a);
    
    /* Update the register */
    value.format = vpiStringVal;
    value.value.str = result_str;
    vpi_put_value(arg_handle, &value, NULL, vpiNoDelay);
    
    /* Cleanup */
    free(result_str);
    fmpz_clear(a);
    
    return 0;
}

/* fmpz_inc_mod PLI implementation */
PLI_INT32 fmpz_inc_mod_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    PLI_INT32 arg_type;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $fmpz_inc_mod requires 3 arguments (result, input, modulus)\n");
        return 1;
    }
    
    while ((arg_handle = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
        arg_type = vpi_get(vpiType, arg_handle);
        if (arg_type != vpiReg && arg_type != vpiMemoryWord) {
            vpi_printf("ERROR: $fmpz_inc_mod arguments must be registers\n");
            return 1;
        }
    }
    
    if (arg_count != 3) {
        vpi_printf("ERROR: $fmpz_inc_mod requires exactly 3 arguments\n");
        return 1;
    }
    
    return 0;
}

PLI_INT32 fmpz_inc_mod_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter;
    vpiHandle result_handle, input_handle, mod_handle;
    s_vpi_value value;
    fmpz_t a, result, mod, temp;
    char *result_str;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    result_handle = vpi_scan(arg_iter);
    input_handle = vpi_scan(arg_iter);
    mod_handle = vpi_scan(arg_iter);
    
    /* Initialize FLINT integers */
    fmpz_init(a);
    fmpz_init(result);
    fmpz_init(mod);
    fmpz_init(temp);
    
    /* Get input value */
    value.format = vpiStringVal;
    vpi_get_value(input_handle, &value);
    if (verilog_str_to_fmpz(a, value.value.str) != 0) {
        goto cleanup;
    }
    
    /* Get modulus value */
    vpi_get_value(mod_handle, &value);
    if (verilog_str_to_fmpz(mod, value.value.str) != 0) {
        goto cleanup;
    }
    
    /* Perform modular increment */
    fmpz_add_ui(temp, a, 1UL);
    fmpz_mod(result, temp, mod);
    
    /* Convert result back to string */
    result_str = fmpz_to_verilog_str(result);
    
    /* Set result in Verilog */
    value.format = vpiStringVal;
    value.value.str = result_str;
    vpi_put_value(result_handle, &value, NULL, vpiNoDelay);
    
    free(result_str);

cleanup:
    fmpz_clear(a);
    fmpz_clear(result);
    fmpz_clear(mod);
    fmpz_clear(temp);
    
    return 0;
}

/* Registration function */
void register_increment_pli(void) {
    s_vpi_systf_data tf_data;
    
    /* Register fmpz_inc */
    tf_data.type = vpiSysTask;
    tf_data.tfname = "$fmpz_inc";
    tf_data.calltf = fmpz_inc_calltf;
    tf_data.compiletf = fmpz_inc_compiletf;
    tf_data.sizetf = NULL;
    tf_data.user_data = NULL;
    vpi_register_systf(&tf_data);
    
    /* Register fmpz_inc_inplace */
    tf_data.tfname = "$fmpz_inc_inplace";
    tf_data.calltf = fmpz_inc_inplace_calltf;
    tf_data.compiletf = fmpz_inc_inplace_compiletf;
    vpi_register_systf(&tf_data);
    
    /* Register fmpz_inc_mod */
    tf_data.tfname = "$fmpz_inc_mod";
    tf_data.calltf = fmpz_inc_mod_calltf;
    tf_data.compiletf = fmpz_inc_mod_compiletf;
    vpi_register_systf(&tf_data);
}

/* VPI startup routine */
void (*vlog_startup_routines[])() = {
    register_increment_pli,
    0
};
