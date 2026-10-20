// div_with_rem_vpi.c
// VPI module for division with remainder (secured mathematical operations)
// For Icarus Verilog

#include "vpi_user.h"
#include <stdlib.h>
#include <string.h>

// Calltf function: executes the division operation
PLI_INT32 div_with_rem_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, dividend_handle, divisor_handle;
    vpiHandle quot_handle, rem_handle;
    s_vpi_value arg_value;
    PLI_INT64 dividend, divisor, quotient, remainder;
    
    // Get handle to system task
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    
    // Get argument iterator
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $div_with_rem requires 4 arguments\n");
        return 0;
    }
    
    // Read dividend (input argument 1)
    dividend_handle = vpi_scan(arg_iter);
    arg_value.format = vpiIntVal;
    vpi_get_value(dividend_handle, &arg_value);
    dividend = arg_value.value.integer;
    
    // Read divisor (input argument 2)
    divisor_handle = vpi_scan(arg_iter);
    vpi_get_value(divisor_handle, &arg_value);
    divisor = arg_value.value.integer;
    
    // Get quotient register handle (output argument 3)
    quot_handle = vpi_scan(arg_iter);
    
    // Get remainder register handle (output argument 4)
    rem_handle = vpi_scan(arg_iter);
    
    vpi_free_object(arg_iter);
    
    // Check for division by zero (security check)
    if (divisor == 0) {
        vpi_printf("ERROR: Division by zero detected in $div_with_rem\n");
        arg_value.format = vpiIntVal;
        arg_value.value.integer = 0;
        vpi_put_value(quot_handle, &arg_value, NULL, vpiNoDelay);
        vpi_put_value(rem_handle, &arg_value, NULL, vpiNoDelay);
        return -1;
    }
    
    // Handle zero dividend
    if (dividend == 0) {
        quotient = 0;
        remainder = 0;
    } else {
        // Perform truncated division (rounds toward zero)
        quotient = dividend / divisor;
        remainder = dividend % divisor;
    }
    
    // Write quotient to output register
    arg_value.format = vpiIntVal;
    arg_value.value.integer = quotient;
    vpi_put_value(quot_handle, &arg_value, NULL, vpiNoDelay);
    
    // Write remainder to output register
    arg_value.value.integer = remainder;
    vpi_put_value(rem_handle, &arg_value, NULL, vpiNoDelay);
    
    vpi_printf("DIV_SECURED: %lld / %lld = %lld remainder %lld\n", 
               dividend, divisor, quotient, remainder);
    
    return 0;
}

// Compiletf function: validates arguments at compile time
PLI_INT32 div_with_rem_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $div_with_rem requires 4 arguments: dividend, divisor, quotient, remainder\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    // Count arguments
    while ((arg_handle = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
    }
    
    if (arg_count != 4) {
        vpi_printf("ERROR: $div_with_rem requires exactly 4 arguments, found %d\n", arg_count);
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    return 0;
}

// Register the system task
void register_div_with_rem() {
    s_vpi_systf_data task_data;
    
    task_data.type = vpiSysTask;
    task_data.sysfunctype = 0;
    task_data.tfname = "$div_with_rem";
    task_data.calltf = div_with_rem_calltf;
    task_data.compiletf = div_with_rem_compiletf;
    task_data.sizetf = NULL;
    task_data.user_data = NULL;
    
    vpi_register_systf(&task_data);
}

// Startup routine array (required by Icarus Verilog)
void (*vlog_startup_routines[])() = {
    register_div_with_rem,
    0  // NULL terminator
};
