//
// mixed_subtraction_pli.c
// Mithril_Cryptography_PLI_Module
//
// Created by Andres Barbudo on 21-08-25.
//

#include "v1.1-4.h"
#include <stdio.h>
#include <stdlib.h>

// Function registration table
static s_vpi_systf_data tf_data[] = {
    {
        .type = vpiSysFunc,
        .tfname = "$mixed_subtract",
        .calltf = mixed_subtraction_calltf,
        .compiletf = mixed_subtraction_compiletf,
        .sizetf = mixed_subtraction_sizetf,
        .user_data = 0
    },
    {0}
};

// Registration function called by simulator
void mixed_subtraction_register(void) {
    s_vpi_systf_data *tf;
    for (tf = tf_data; tf->type; tf++) {
        vpi_register_systf(tf);
    }
}

// Compile-time function checking
PLI_INT32 mixed_subtraction_compiletf(PLI_BYTE8* user_data) {
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (!arg_iter) {
        vpi_printf("ERROR: $mixed_subtract requires exactly 2 arguments\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    int arg_count = 0;
    vpiHandle arg_handle;
    while ((arg_handle = vpi_scan(arg_iter))) {
        arg_count++;
    }
    
    if (arg_count != 2) {
        vpi_printf("ERROR: $mixed_subtract requires exactly 2 arguments, got %d\n", arg_count);
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    return 0;
}

// Runtime function execution
PLI_INT32 mixed_subtraction_calltf(PLI_BYTE8* user_data) {
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (!arg_iter) {
        vpi_printf("ERROR: $mixed_subtract execution failed - no arguments\n");
        return 0;
    }
    
    // Get arguments
    vpiHandle arg1 = vpi_scan(arg_iter);  // Large integer (fmpz)
    vpiHandle arg2 = vpi_scan(arg_iter);  // Unsigned short
    
    // Initialize FLINT integers
    fmpz_t a, result;
    fmpz_init(a);
    fmpz_init(result);
    
    // Convert Verilog values to FLINT format
    if (convert_verilog_to_fmpz(arg1, a) != 0) {
        vpi_printf("ERROR: Failed to convert first argument to FLINT integer\n");
        fmpz_clear(a);
        fmpz_clear(result);
        return 0;
    }
    
    // Get unsigned short value
    s_vpi_value value_s;
    value_s.format = vpiIntVal;
    vpi_get_value(arg2, &value_s);
    unsigned short b = (unsigned short)value_s.value.integer;
    
    // Perform mixed subtraction
    fmpz_t temp;
    fmpz_init(temp);
    fmpz_set_ui(temp, (unsigned long)b);
    fmpz_sub(result, a, temp);
    fmpz_clear(temp);
    
    // Convert result back to Verilog format
    s_vpi_value result_value;
    char result_str[1024];
    fmpz_get_str(result_str, 10, result);
    
    result_value.format = vpiStringVal;
    result_value.value.str = result_str;
    vpi_put_value(systf_handle, &result_value, NULL, vpiNoDelay);
    
    // Cleanup
    fmpz_clear(a);
    fmpz_clear(result);
    
    return 0;
}

// Function to determine return size
PLI_INT32 mixed_subtraction_sizetf(PLI_BYTE8* user_data) {
    return 512;  // Return large enough size for big integers
}

// Helper function to convert Verilog value to FLINT integer
int convert_verilog_to_fmpz(vpiHandle handle, fmpz_t result) {
    s_vpi_value value_s;
    value_s.format = vpiStringVal;
    vpi_get_value(handle, &value_s);
    
    if (fmpz_set_str(result, value_s.value.str, 10) != 0) {
        return -1;  // Conversion failed
    }
    
    return 0;  // Success
}

// Helper function to convert FLINT integer to Verilog value
int convert_fmpz_to_verilog(fmpz_t value, vpiHandle handle) {
    char str_buffer[1024];
    fmpz_get_str(str_buffer, 10, value);
    
    s_vpi_value vpi_value;
    vpi_value.format = vpiStringVal;
    vpi_value.value.str = str_buffer;
    
    vpi_put_value(handle, &vpi_value, NULL, vpiNoDelay);
    return 0;
}

// VPI startup function
void (*vlog_startup_routines[])(void) = {
    mixed_subtraction_register,
    0
};
