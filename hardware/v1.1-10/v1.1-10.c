// v1.1-10.c
// Karatsuba multiplication PLI implementation for IoT devices

#include "v1.1-10.h"

// Simplified Karatsuba implementation for hardware simulation
// This replaces FLINT dependency for Verilog simulation
void karatsuba_multiply(unsigned long long a, unsigned long long b, 
                       unsigned long long* result_high, 
                       unsigned long long* result_low) {
    
    // For demonstration - simplified 64-bit Karatsuba
    if (a == 0 || b == 0) {
        *result_high = 0;
        *result_low = 0;
        return;
    }
    
    // Base case for small numbers
    if (a < 0x10000 && b < 0x10000) {
        unsigned long long product = a * b;
        *result_high = product >> 32;
        *result_low = product & 0xFFFFFFFF;
        return;
    }
    
    // Split operands for Karatsuba algorithm
    int half_bits = 32;
    unsigned long long mask = 0xFFFFFFFF;
    
    unsigned long long a_high = a >> half_bits;
    unsigned long long a_low = a & mask;
    unsigned long long b_high = b >> half_bits;
    unsigned long long b_low = b & mask;
    
    // Three multiplications (Karatsuba optimization)
    unsigned long long z0_h, z0_l, z2_h, z2_l, z1_h, z1_l;
    
    karatsuba_multiply(a_low, b_low, &z0_h, &z0_l);     // z0
    karatsuba_multiply(a_high, b_high, &z2_h, &z2_l);   // z2
    karatsuba_multiply(a_low + a_high, b_low + b_high, &z1_h, &z1_l); // z1
    
    // z1 = z1 - z2 - z0 (middle term)
    unsigned long long z1_result = (z1_h << 32) + z1_l - 
                                   (z2_h << 32) - z2_l - 
                                   (z0_h << 32) - z0_l;
    
    // Combine results: result = z2 * 2^(2*half_bits) + z1 * 2^half_bits + z0
    unsigned long long final_result = ((z2_h << 32) + z2_l) * 
                                     (1ULL << (2 * half_bits)) + 
                                     z1_result * (1ULL << half_bits) + 
                                     (z0_h << 32) + z0_l;
    
    *result_high = final_result >> 32;
    *result_low = final_result & 0xFFFFFFFF;
}

// VPI system function implementation
PLI_INT32 karats_mul_calltf(PLI_BYTE8* user_data) {
    vpiHandle systf_handle, arg_iterator, arg_handle;
    s_vpi_value arg_value, result_value;
    
    unsigned long long operand_a = 0, operand_b = 0;
    unsigned long long result_high = 0, result_low = 0;
    
    // Get system function handle
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $karats_mul requires two arguments\n");
        return 0;
    }
    
    // Get first argument (operand A)
    arg_handle = vpi_scan(arg_iterator);
    if (arg_handle) {
        arg_value.format = vpiIntVal;
        vpi_get_value(arg_handle, &arg_value);
        operand_a = (unsigned long long)arg_value.value.integer;
    }
    
    // Get second argument (operand B)
    arg_handle = vpi_scan(arg_iterator);
    if (arg_handle) {
        arg_value.format = vpiIntVal;
        vpi_get_value(arg_handle, &arg_value);
        operand_b = (unsigned long long)arg_value.value.integer;
    }
    
    vpi_free_object(arg_iterator);
    
    // Perform Karatsuba multiplication
    karatsuba_multiply(operand_a, operand_b, &result_high, &result_low);
    
    // Return lower 32 bits as function result
    result_value.format = vpiIntVal;
    result_value.value.integer = (PLI_INT32)result_low;
    vpi_put_value(systf_handle, &result_value, NULL, vpiNoDelay);
    
    // Print secure operation log for IoT monitoring
    vpi_printf("SECURE_MUL: %llu * %llu = %llu%08llu (Karatsuba)\n", 
               operand_a, operand_b, result_high, result_low);
    
    return 0;
}

// Compile-time checking
PLI_INT32 karats_mul_compiletf(PLI_BYTE8* user_data) {
    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_count = 0;
    
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    
    if (arg_iterator != NULL) {
        while ((arg_handle = vpi_scan(arg_iterator)) != NULL) {
            arg_count++;
        }
    }
    
    if (arg_count != 2) {
        vpi_printf("ERROR: $karats_mul requires exactly 2 arguments, got %d\n", 
                   arg_count);
        return -1;
    }
    
    return 0;
}

// Return size for function result
PLI_INT32 karats_mul_sizetf(PLI_BYTE8* user_data) {
    return 32; // Return 32-bit result
}

// Registration function
void register_karats_systfs(void) {
    s_vpi_systf_data task_data;
    
    // Register $karats_mul function
    task_data.type = vpiSysFunc;
    task_data.sysfunctype = vpiIntFunc;
    task_data.tfname = "$karats_mul";
    task_data.calltf = karats_mul_calltf;
    task_data.compiletf = karats_mul_compiletf;
    task_data.sizetf = karats_mul_sizetf;
    task_data.user_data = NULL;
    
    vpi_register_systf(&task_data);
}

// Required startup function for Icarus Verilog
void (*vlog_startup_routines[])(void) = {
    register_karats_systfs,
    NULL
};
