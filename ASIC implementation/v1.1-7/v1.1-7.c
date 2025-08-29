//
// multiplication_pli.c  
// Mithril_Cryptography_API PLI Interface
//
// Secure multiplication for IoT devices in Verilog simulation
//

#include "multiplication_pli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Size function - determines return value bit width
PLI_INT32 secure_mul_sizetf(PLI_BYTE8 *user_data) {
    // Return 128-bit result to handle large multiplications
    return 128;
}

// Check function - validates arguments at compile time
PLI_INT32 secure_mul_checktf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_type;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $secure_mul requires exactly 2 arguments\n");
        return -1;
    }
    
    // Check each argument
    while ((arg_handle = vpi_scan(arg_iterator))) {
        arg_count++;
        arg_type = vpi_get(vpiType, arg_handle);
        
        if (arg_type != vpiConstant && arg_type != vpiNet && 
            arg_type != vpiReg && arg_type != vpiParameter) {
            vpi_printf("ERROR: $secure_mul argument %d must be a numeric value\n", arg_count);
            return -1;
        }
        
        if (arg_count > 2) {
            vpi_printf("ERROR: $secure_mul accepts maximum 2 arguments\n");
            return -1;
        }
    }
    
    if (arg_count != 2) {
        vpi_printf("ERROR: $secure_mul requires exactly 2 arguments, got %d\n", arg_count);
        return -1;
    }
    
    return 0;
}

// Main function - performs the secure multiplication
PLI_INT32 secure_mul_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iterator, arg_handle;
    s_vpi_value arg_value;
    fmpz_t a, b, result;
    char result_str[256];
    s_vpi_value return_value;
    PLI_UINT64 verilog_a, verilog_b;
    int status;
    
    // Initialize FLINT integers
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(result);
    
    // Get system task handle and arguments
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $secure_mul - no arguments found\n");
        fmpz_clear(a);
        fmpz_clear(b);
        fmpz_clear(result);
        return -1;
    }
    
    // Get first argument
    arg_handle = vpi_scan(arg_iterator);
    arg_value.format = vpiIntVal;
    vpi_get_value(arg_handle, &arg_value);
    verilog_a = (PLI_UINT64)arg_value.value.integer;
    
    // Get second argument
    arg_handle = vpi_scan(arg_iterator);
    arg_value.format = vpiIntVal;
    vpi_get_value(arg_handle, &arg_value);
    verilog_b = (PLI_UINT64)arg_value.value.integer;
    
    vpi_free_object(arg_iterator);
    
    // Convert Verilog values to FLINT integers
    fmpz_set_ui(a, verilog_a);
    fmpz_set_ui(b, verilog_b);
    
    // Perform secure multiplication
    status = fmpz_mul_safe(result, a, b);
    
    if (status != 0) {
        vpi_printf("ERROR: $secure_mul - multiplication failed with status %d\n", status);
        fmpz_clear(a);
        fmpz_clear(b);
        fmpz_clear(result);
        return -1;
    }
    
    // Convert result back to Verilog format
    // For IoT applications, we'll limit to 64-bit results for practical use
    PLI_UINT64 result_val = fmpz_get_ui(result);
    
    // Log the operation for IoT security audit
    vpi_printf("IoT Secure Multiplication: %llu * %llu = %llu\n", 
               verilog_a, verilog_b, result_val);
    
    // Set return value
    return_value.format = vpiIntVal;
    return_value.value.integer = (PLI_INT32)result_val;
    vpi_put_value(systf_handle, &return_value, NULL, vpiNoDelay);
    
    // Clean up
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(result);
    
    return 0;
}

// Registration function
void multiplication_pli_register(void) {
    s_vpi_systf_data systf_data;
    
    systf_data.type = vpiSysFunc;
    systf_data.tfname = "$secure_mul";
    systf_data.calltf = secure_mul_calltf;
    systf_data.checktf = secure_mul_checktf;
    systf_data.sizetf = secure_mul_sizetf;
    systf_data.user_data = NULL;
    
    vpi_register_systf(&systf_data);
    
    vpi_printf("IoT Secure Multiplication PLI module registered\n");
}

// Required PLI registration entry point
void (*vlog_startup_routines[])(void) = {
    multiplication_pli_register,
    0
};
