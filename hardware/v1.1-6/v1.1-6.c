//
// fmpz_decrement_pli.c
// PLI Module for Icarus Verilog - Secure Decrement
//
// Created by Andres Barbudo on 21-08-25.
//

#include "fmpz_decrement_pli.h"

// Internal secure decrement function
fmpz_error_t secure_dec(PLI_INT32 *value, PLI_INT32 width) {
    PLI_INT32 max_val = (1 << width) - 1;
    
    // Check for underflow - if value is zero, handle securely
    if (*value == 0) {
        // Set to maximum value for unsigned arithmetic simulation
        *value = max_val;
        vpi_printf("WARNING: Underflow detected in secure_decrement - wrapping to max value\n");
        return E_FMPZ_UFL;
    }
    
    // Simply decrement by 1
    (*value)--;
    return E_FMPZ_OK;
}

// Compile time function - validates arguments
PLI_INT32 secure_decrement_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_itr, arg_handle;
    PLI_INT32 arg_type;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_itr = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_itr == NULL) {
        vpi_printf("ERROR: $secure_decrement requires at least one argument\n");
        return 0;
    }
    
    // Check first argument (the value to decrement)
    arg_handle = vpi_scan(arg_itr);
    arg_type = vpi_get(vpiType, arg_handle);
    
    if (arg_type != vpiReg && arg_type != vpiNet) {
        vpi_printf("ERROR: $secure_decrement first argument must be a reg or net\n");
        vpi_free_object(arg_itr);
        return 0;
    }
    
    vpi_free_object(arg_itr);
    return 1;
}

// Runtime function - performs the actual decrement
PLI_INT32 secure_decrement_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_itr, arg_handle;
    s_vpi_value arg_val;
    PLI_INT32 width;
    fmpz_error_t result;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_itr = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_itr == NULL) {
        vpi_printf("ERROR: $secure_decrement - no arguments found\n");
        return 0;
    }
    
    // Get the first argument (value to decrement)
    arg_handle = vpi_scan(arg_itr);
    width = vpi_get(vpiSize, arg_handle);
    
    // Read current value
    arg_val.format = vpiIntVal;
    vpi_get_value(arg_handle, &arg_val);
    
    // Perform secure decrement
    PLI_INT32 current_val = arg_val.value.integer;
    result = secure_dec(&current_val, width);
    
    // Write back the result
    arg_val.value.integer = current_val;
    vpi_put_value(arg_handle, &arg_val, NULL, vpiNoDelay);
    
    vpi_free_object(arg_itr);
    
    // Return error code through $secure_decrement function
    s_vpi_value return_val;
    return_val.format = vpiIntVal;
    return_val.value.integer = result;
    vpi_put_value(systf_handle, &return_val, NULL, vpiNoDelay);
    
    return 0;
}

// Size function - returns 32 bits for return value
PLI_INT32 secure_decrement_sizetf(PLI_BYTE8 *user_data) {
    return 32;
}

// Registration function
void register_secure_decrement() {
    s_vpi_systf_data tf_data;
    
    tf_data.type = vpiSysFunc;
    tf_data.sysfunctype = vpiIntFunc;
    tf_data.tfname = "$secure_decrement";
    tf_data.calltf = secure_decrement_calltf;
    tf_data.compiletf = secure_decrement_compiletf;
    tf_data.sizetf = secure_decrement_sizetf;
    tf_data.user_data = NULL;
    
    vpi_register_systf(&tf_data);
}

// PLI initialization
void (*vlog_startup_routines[])() = {
    register_secure_decrement,
    0
};
