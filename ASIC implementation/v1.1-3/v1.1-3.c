//
// mixed_addition_pli.c
// Mithril_Cryptography_API PLI Module
//
// Created by Andres Barbudo on 28-08-25.
//

#include "mixed_addition_pli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compile-time function to check arguments
PLI_INT32 mixed_addition_compiletf(PLI_BYTE8* user_data)
{
    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_type;
    int arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $mixed_addition requires exactly 3 arguments\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    // Check argument count and types
    while ((arg_handle = vpi_scan(arg_iterator))) {
        arg_count++;
        arg_type = vpi_get(vpiType, arg_handle);
        
        switch (arg_count) {
            case 1: // Output result (should be reg)
                if (arg_type != vpiReg && arg_type != vpiMemoryWord) {
                    vpi_printf("ERROR: First argument (result) must be a reg\n");
                    vpi_control(vpiFinish, 1);
                    return 0;
                }
                break;
            case 2: // Large integer string input
                if (arg_type != vpiReg && arg_type != vpiMemoryWord) {
                    vpi_printf("ERROR: Second argument (large_int) must be a string\n");
                    vpi_control(vpiFinish, 1);
                    return 0;
                }
                break;
            case 3: // Small integer input
                if (arg_type != vpiReg && arg_type != vpiIntegerVar && arg_type != vpiParameter) {
                    vpi_printf("ERROR: Third argument (small_int) must be an integer\n");
                    vpi_control(vpiFinish, 1);
                    return 0;
                }
                break;
        }
    }
    
    if (arg_count != 3) {
        vpi_printf("ERROR: $mixed_addition requires exactly 3 arguments (result, large_int_string, small_int)\n");
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    return 0;
}

// Runtime function to perform the mixed addition
PLI_INT32 mixed_addition_calltf(PLI_BYTE8* user_data)
{
    vpiHandle systf_handle, arg_iterator;
    vpiHandle result_handle, large_int_handle, small_int_handle;
    s_vpi_value value_s;
    fmpz_t large_num, result_num;
    unsigned long small_num;
    char *large_int_str, *result_str;
    
    // Get system function handle
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    // Get arguments
    result_handle = vpi_scan(arg_iterator);      // Output result
    large_int_handle = vpi_scan(arg_iterator);   // Large integer string
    small_int_handle = vpi_scan(arg_iterator);   // Small integer
    
    // Initialize FLINT integers
    fmpz_init(large_num);
    fmpz_init(result_num);
    
    // Get large integer string
    value_s.format = vpiStringVal;
    vpi_get_value(large_int_handle, &value_s);
    large_int_str = value_s.value.str;
    
    // Get small integer
    value_s.format = vpiIntVal;
    vpi_get_value(small_int_handle, &value_s);
    small_num = (unsigned long)value_s.value.integer;
    
    // Perform FLINT operations
    if (fmpz_set_str(large_num, large_int_str, 10) != 0) {
        vpi_printf("ERROR: Invalid large integer string format\n");
        fmpz_clear(large_num);
        fmpz_clear(result_num);
        return 0;
    }
    
    // Perform mixed addition using FLINT
    fmpz_add_ui(result_num, large_num, small_num);
    
    // Convert result to string
    result_str = fmpz_get_str(NULL, 10, result_num);
    
    // Set the result back to Verilog
    value_s.format = vpiStringVal;
    value_s.value.str = result_str;
    vpi_put_value(result_handle, &value_s, NULL, vpiNoDelay);
    
    // Debug output
    vpi_printf("Mixed Addition: %s + %lu = %s\n", large_int_str, small_num, result_str);
    
    // Clean up
    free(result_str);
    fmpz_clear(large_num);
    fmpz_clear(result_num);
    
    return 0;
}

// Registration function for PLI
void mixed_addition_register(void)
{
    s_vpi_systf_data tf_data;
    
    tf_data.type = vpiSysTask;
    tf_data.tfname = "$mixed_addition";
    tf_data.calltf = mixed_addition_calltf;
    tf_data.compiletf = mixed_addition_compiletf;
    tf_data.sizetf = NULL;
    tf_data.user_data = NULL;
    
    vpi_register_systf(&tf_data);
}

// Registration table (required by Icarus Verilog)
void (*vlog_startup_routines[])(void) = {
    mixed_addition_register,
    0
};
