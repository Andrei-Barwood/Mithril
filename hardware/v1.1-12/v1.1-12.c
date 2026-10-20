#include <stdio.h>
#include <stdlib.h>
#include <vpi_user.h>
#include <string.h>

// Calltf routine: performs the actual modulo 2^k operation
static PLI_INT32 rem_mod_pow_of_2_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value arg_value;
    PLI_INT64 x_val, k_val, result;
    PLI_UINT64 modulus;
    
    // Get system task handle
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    
    // Get argument iterator
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $rem_mod_pow_of_2 requires 3 arguments\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    // Get first argument: x (input value)
    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiIntVal;
    vpi_get_value(arg_handle, &arg_value);
    x_val = arg_value.value.integer;
    
    // Get second argument: k (power)
    arg_handle = vpi_scan(arg_iter);
    vpi_get_value(arg_handle, &arg_value);
    k_val = arg_value.value.integer;
    
    // Validate k value for security
    if (k_val < 0 || k_val > 63) {
        vpi_printf("ERROR: k must be in range [0, 63] for secure operation\n");
        vpi_free_object(arg_iter);
        return 0;
    }
    
    // Calculate modulus = 2^k using bit shift (secure method)
    modulus = (k_val == 63) ? ((PLI_UINT64)1 << 63) : ((PLI_UINT64)1 << k_val);
    
    // Perform modulo operation: result = x % (2^k)
    // Using bitwise AND for efficiency: x & (2^k - 1)
    if (x_val >= 0) {
        result = x_val & (modulus - 1);
    } else {
        // Handle negative values properly for modulo
        result = x_val % (PLI_INT64)modulus;
        if (result < 0) result += modulus;
    }
    
    // Get third argument: result (output register)
    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiIntVal;
    arg_value.value.integer = (PLI_INT32)result;
    vpi_put_value(arg_handle, &arg_value, NULL, vpiNoDelay);
    
    vpi_free_object(arg_iter);
    return 0;
}

// Compiletf routine: validates arguments at compile time
static PLI_INT32 rem_mod_pow_of_2_compiletf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iter == NULL) {
        vpi_printf("ERROR: $rem_mod_pow_of_2 requires exactly 3 arguments\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    // Count and validate arguments
    while ((arg_handle = vpi_scan(arg_iter)) != NULL) {
        arg_count++;
    }
    
    if (arg_count != 3) {
        vpi_printf("ERROR: $rem_mod_pow_of_2 requires exactly 3 arguments (x, k, result)\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    return 0;
}

// Registration function
void rem_mod_pow_of_2_register(void) {
    s_vpi_systf_data task_data;
    
    task_data.type = vpiSysTask;
    task_data.tfname = "$rem_mod_pow_of_2";
    task_data.calltf = rem_mod_pow_of_2_calltf;
    task_data.compiletf = rem_mod_pow_of_2_compiletf;
    task_data.sizetf = NULL;
    task_data.user_data = NULL;
    
    vpi_register_systf(&task_data);
}

// Startup routines array for Icarus Verilog
void (*vlog_startup_routines[])(void) = {
    rem_mod_pow_of_2_register,
    0  // Must be null-terminated
};
