//
// 
// Verilog PLI Interface to libsodium
// Secure bridge between hardware simulation and libsodium
//

#include "mithril_pli_libsodium.h"
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//===========================================================================
// Initialization
//===========================================================================
static int libsodium_initialized = 0;

static int ensure_libsodium_init(void) {
    if (!libsodium_initialized) {
        if (sodium_init() < 0) {
            return -1;
        }
        libsodium_initialized = 1;
    }
    return 0;
}

//===========================================================================
// PLI Task: $mithril_add_scalar
// Secure 256-bit addition using libsodium principles
//===========================================================================
PLI_INT32 mithril_add_scalar_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value arg_value;
    
    uint8_t a[32], b[32], result[32];
    int overflow;
    
    // Initialize libsodium
    if (ensure_libsodium_init() < 0) {
        vpi_printf("ERROR: libsodium initialization failed\n");
        return 0;
    }
    
    // Get system task handle
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (!arg_iter) {
        vpi_printf("ERROR: $mithril_add_scalar requires 3 arguments\n");
        return 0;
    }
    
    // Get operand A (256-bit vector)
    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiVectorVal;
    vpi_get_value(arg_handle, &arg_value);
    
    // Convert from Verilog vector to byte array (little-endian)
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        a[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }
    
    // Get operand B
    arg_handle = vpi_scan(arg_iter);
    vpi_get_value(arg_handle, &arg_value);
    
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        b[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }
    
    // Perform constant-time addition
    overflow = 0;
    uint16_t carry = 0;
    
    for (size_t i = 0; i < 32; i++) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        result[i] = (uint8_t)(sum & 0xFF);
        carry = sum >> 8;
    }
    
    if (carry != 0) {
        overflow = 1;
    }
    
    // Get result handle
    arg_handle = vpi_scan(arg_iter);
    
    // Convert result back to Verilog vector
    s_vpi_vecval result_vec[8];  // 256 bits = 8 x 32-bit words
    
    for (int i = 0; i < 8; i++) {
        result_vec[i].aval = 0;
        result_vec[i].bval = 0;
        
        for (int j = 0; j < 4; j++) {
            int byte_idx = i * 4 + j;
            result_vec[i].aval |= ((uint32_t)result[byte_idx]) << (j * 8);
        }
    }
    
    arg_value.format = vpiVectorVal;
    arg_value.value.vector = result_vec;
    vpi_put_value(arg_handle, &arg_value, NULL, vpiNoDelay);
    
    // Cleanup iterator
    vpi_free_object(arg_iter);
    
    // Secure cleanup
    sodium_memzero(a, sizeof(a));
    sodium_memzero(b, sizeof(b));
    sodium_memzero(result, sizeof(result));
    
    vpi_printf("INFO: Scalar addition completed (overflow=%d)\n", overflow);
    
    return 0;
}

//===========================================================================
// PLI Task: $mithril_add_mod_curve25519
// Modular addition using libsodium's Ed25519 scalar operations
//===========================================================================
PLI_INT32 mithril_add_mod_curve25519_calltf(PLI_BYTE8 *user_data) {
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value arg_value;
    
    uint8_t a[crypto_core_ed25519_SCALARBYTES];
    uint8_t b[crypto_core_ed25519_SCALARBYTES];
    uint8_t result[crypto_core_ed25519_SCALARBYTES];
    
    // Initialize libsodium
    if (ensure_libsodium_init() < 0) {
        vpi_printf("ERROR: libsodium initialization failed\n");
        return 0;
    }
    
    // Get arguments (similar to above)
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    
    if (!arg_iter) {
        vpi_printf("ERROR: $mithril_add_mod_curve25519 requires 3 arguments\n");
        return 0;
    }
    
    // Get operand A
    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiVectorVal;
    vpi_get_value(arg_handle, &arg_value);
    
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        a[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }
    
    // Get operand B
    arg_handle = vpi_scan(arg_iter);
    vpi_get_value(arg_handle, &arg_value);
    
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        b[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }
    
    // Perform modular addition using libsodium (timing-safe!)
    crypto_core_ed25519_scalar_add(result, a, b);
    
    // Get result handle and write back
    arg_handle = vpi_scan(arg_iter);
    
    s_vpi_vecval result_vec[8];
    for (int i = 0; i < 8; i++) {
        result_vec[i].aval = 0;
        result_vec[i].bval = 0;
        
        for (int j = 0; j < 4; j++) {
            int byte_idx = i * 4 + j;
            result_vec[i].aval |= ((uint32_t)result[byte_idx]) << (j * 8);
        }
    }
    
    arg_value.format = vpiVectorVal;
    arg_value.value.vector = result_vec;
    vpi_put_value(arg_handle, &arg_value, NULL, vpiNoDelay);
    
    vpi_free_object(arg_iter);
    
    // Secure cleanup
    sodium_memzero(a, sizeof(a));
    sodium_memzero(b, sizeof(b));
    sodium_memzero(result, sizeof(result));
    
    vpi_printf("INFO: Curve25519 modular addition completed\n");
    
    return 0;
}

//===========================================================================
// PLI Registration
//===========================================================================
void register_mithril_pli_tasks(void) {
    s_vpi_systf_data tf_data;
    
    // Register $mithril_add_scalar
    tf_data.type      = vpiSysTask;
    tf_data.sysfunctype = 0;
    tf_data.tfname    = "$mithril_add_scalar";
    tf_data.calltf    = mithril_add_scalar_calltf;
    tf_data.compiletf = NULL;
    tf_data.sizetf    = NULL;
    tf_data.user_data = NULL;
    vpi_register_systf(&tf_data);
    
    // Register $mithril_add_mod_curve25519
    tf_data.tfname    = "$mithril_add_mod_curve25519";
    tf_data.calltf    = mithril_add_mod_curve25519_calltf;
    vpi_register_systf(&tf_data);
    
    vpi_printf("INFO: Mithril libsodium PLI tasks registered\n");
}

// VPI startup routine
void (*vlog_startup_routines[])(void) = {
    register_mithril_pli_tasks,
    0
};
