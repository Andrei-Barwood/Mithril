// subtraction_pli.c
// PLI Module for Secure Subtraction Operations
// Mithril_Cryptography_API - Icarus Verilog Interface

#include "subtraction_pli.h"

// Secure subtraction function adapted for PLI
int secure_subtract_with_underflow(long long *result, long long a, long long b) {
    int underflow = E_FLINT_OK;
    
    // Check if a < b (would cause underflow in unsigned arithmetic)
    if (a < b) {
        // Handle underflow case: compute |b - a| - 1
        // This mimics the original CLINT behavior
        *result = (b - a) - 1;
        underflow = E_FLINT_UFL;
    } else {
        // Normal case: a >= b
        *result = a - b;
    }
    
    return underflow;
}

// PLI calltf function - main execution logic
PLI_INT32 secure_subtraction_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_h, arg_iter, arg_h;
    s_vpi_value arg_val;
    long long operand_a, operand_b, result;
    int underflow_flag;
    
    // Get system task handle
    systf_h = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_h);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $secure_subtract requires 4 arguments: a, b, result, underflow_flag\n");
        return 0;
    }
    
    // Get first argument (operand a)
    arg_h = vpi_scan(arg_iter);
    arg_val.format = vpiIntVal;
    vpi_get_value(arg_h, &arg_val);
    operand_a = arg_val.value.integer;
    
    // Get second argument (operand b)
    arg_h = vpi_scan(arg_iter);
    vpi_get_value(arg_h, &arg_val);
    operand_b = arg_val.value.integer;
    
    // Perform secure subtraction
    underflow_flag = secure_subtract_with_underflow(&result, operand_a, operand_b);
    
    // Set result in third argument
    arg_h = vpi_scan(arg_iter);
    arg_val.format = vpiIntVal;
    arg_val.value.integer = (PLI_INT32)result;
    vpi_put_value(arg_h, &arg_val, NULL, vpiNoDelay);
    
    // Set underflow flag in fourth argument
    arg_h = vpi_scan(arg_iter);
    arg_val.value.integer = underflow_flag;
    vpi_put_value(arg_h, &arg_val, NULL, vpiNoDelay);
    
    // Clean up iterator
    vpi_free_object(arg_iter);
    
    // Log operation for security audit
    vpi_printf("SECURITY LOG: Subtraction %lld - %lld = %lld (underflow: %s)\n", 
               operand_a, operand_b, result, 
               underflow_flag ? "YES" : "NO");
    
    return 0;
}

// PLI compiletf function - compile-time checking
PLI_INT32 secure_subtraction_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_h, arg_iter, arg_h;
    int arg_count = 0;
    
    systf_h = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_h);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $secure_subtract requires exactly 4 arguments\n");
        return 0;
    }
    
    // Count arguments
    while ((arg_h = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
    }
    
    if (arg_count != 4) {
        vpi_printf("ERROR: $secure_subtract requires exactly 4 arguments, got %d\n", arg_count);
        return 0;
    }
    
    return 0;
}

// PLI sizetf function - return value sizing
PLI_INT32 secure_subtraction_sizetf(PLI_BYTE8 *user_data) {
    return 32; // 32-bit return value
}

// Registration function
void register_subtraction_pli(void) {
    s_vpi_systf_data tf_data;
    
    tf_data.type = vpiSysTask;
    tf_data.tfname = "$secure_subtract";
    tf_data.calltf = secure_subtraction_calltf;
    tf_data.compiletf = secure_subtraction_compiletf;
    tf_data.sizetf = secure_subtraction_sizetf;
    tf_data.user_data = NULL;
    
    vpi_register_systf(&tf_data);
}

// VPI registration function (required by Icarus Verilog)
void (*vlog_startup_routines[])(void) = {
    register_subtraction_pli,
    0
};
