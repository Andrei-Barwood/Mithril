// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_vpi.c
// Description: Icarus Verilog VPI Module providing co-simulation against
//              libsodium reference implementations.
// System tasks provided:
//   $mithril_add_scalar(a, b, result)
//   $mithril_add_mod_curve25519(a, b, result)
//   $div_with_rem(dividend, divisor, quotient, remainder)
// ============================================================================

#include "vpi_user.h"
#include <sodium.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int libsodium_initialized = 0;

static int ensure_libsodium_init(void) {
    if (!libsodium_initialized) {
        if (sodium_init() < 0) {
            vpi_printf("ERROR: [Mithril VPI] libsodium initialization failed!\n");
            return -1;
        }
        libsodium_initialized = 1;
        vpi_printf("INFO: [Mithril VPI] libsodium initialized successfully\n");
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Task: $mithril_add_scalar(a, b, result)
// ----------------------------------------------------------------------------
static PLI_INT32 mithril_add_scalar_calltf(PLI_BYTE8 *user_data) {
    (void)user_data;
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value arg_value;

    uint8_t a[32] = {0};
    uint8_t b[32] = {0};
    uint8_t result[32] = {0};

    if (ensure_libsodium_init() < 0) return 0;

    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    if (!arg_iter) return 0;

    // Arg 1: operand A (256 bits)
    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiVectorVal;
    vpi_get_value(arg_handle, &arg_value);
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        a[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }

    // Arg 2: operand B (256 bits)
    arg_handle = vpi_scan(arg_iter);
    vpi_get_value(arg_handle, &arg_value);
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        b[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }

    // Reference addition with carry
    uint16_t carry = 0;
    for (int i = 0; i < 32; i++) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        result[i] = (uint8_t)(sum & 0xFF);
        carry = (sum >> 8) & 0x01;
    }

    // Arg 3: output result
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
    return 0;
}

// ----------------------------------------------------------------------------
// Task: $mithril_add_mod_curve25519(a, b, result)
// ----------------------------------------------------------------------------
static PLI_INT32 mithril_add_mod_curve25519_calltf(PLI_BYTE8 *user_data) {
    (void)user_data;
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value arg_value;

    uint8_t a[32] = {0};
    uint8_t b[32] = {0};
    uint8_t result[32] = {0};

    if (ensure_libsodium_init() < 0) return 0;

    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    if (!arg_iter) return 0;

    arg_handle = vpi_scan(arg_iter);
    arg_value.format = vpiVectorVal;
    vpi_get_value(arg_handle, &arg_value);
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        a[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }

    arg_handle = vpi_scan(arg_iter);
    vpi_get_value(arg_handle, &arg_value);
    for (int i = 0; i < 32; i++) {
        int word_idx = i / 4;
        int byte_idx = i % 4;
        b[i] = (arg_value.value.vector[word_idx].aval >> (byte_idx * 8)) & 0xFF;
    }

    // Modular addition over Curve25519 order using libsodium
    crypto_core_ed25519_scalar_add(result, a, b);

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
    return 0;
}

// ----------------------------------------------------------------------------
// Task: $div_with_rem(dividend, divisor, quotient, remainder)
// ----------------------------------------------------------------------------
static PLI_INT32 div_with_rem_calltf(PLI_BYTE8 *user_data) {
    (void)user_data;
    vpiHandle systf_handle, arg_iter, arg_handle;
    s_vpi_value val;

    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_iter = vpi_iterate(vpiArgument, systf_handle);
    if (!arg_iter) return 0;

    // Arg 1: Dividend
    arg_handle = vpi_scan(arg_iter);
    val.format = vpiDecStrVal;
    vpi_get_value(arg_handle, &val);
    uint64_t dividend = strtoull(val.value.str, NULL, 10);

    // Arg 2: Divisor
    arg_handle = vpi_scan(arg_iter);
    val.format = vpiDecStrVal;
    vpi_get_value(arg_handle, &val);
    uint64_t divisor = strtoull(val.value.str, NULL, 10);

    uint64_t quotient = 0;
    uint64_t remainder = 0;
    if (divisor != 0) {
        quotient = dividend / divisor;
        remainder = dividend % divisor;
    } else {
        vpi_printf("WARNING: [Mithril VPI] Division by zero requested!\n");
    }

    // Arg 3: Quotient output
    arg_handle = vpi_scan(arg_iter);
    val.format = vpiDecStrVal;
    char q_str[32], r_str[32];
    snprintf(q_str, sizeof(q_str), "%llu", (unsigned long long)quotient);
    snprintf(r_str, sizeof(r_str), "%llu", (unsigned long long)remainder);

    val.value.str = q_str;
    vpi_put_value(arg_handle, &val, NULL, vpiNoDelay);

    // Arg 4: Remainder output
    arg_handle = vpi_scan(arg_iter);
    val.value.str = r_str;
    vpi_put_value(arg_handle, &val, NULL, vpiNoDelay);

    vpi_free_object(arg_iter);
    return 0;
}

// ----------------------------------------------------------------------------
// VPI System Task Registration
// ----------------------------------------------------------------------------
void register_mithril_vpi_tasks(void) {
    s_vpi_systf_data tf_data;

    memset(&tf_data, 0, sizeof(tf_data));
    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$mithril_add_scalar";
    tf_data.calltf    = mithril_add_scalar_calltf;
    vpi_register_systf(&tf_data);

    memset(&tf_data, 0, sizeof(tf_data));
    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$mithril_add_mod_curve25519";
    tf_data.calltf    = mithril_add_mod_curve25519_calltf;
    vpi_register_systf(&tf_data);

    memset(&tf_data, 0, sizeof(tf_data));
    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$div_with_rem";
    tf_data.calltf    = div_with_rem_calltf;
    vpi_register_systf(&tf_data);

    vpi_printf("INFO: [Mithril VPI] Registered $mithril_add_scalar, $mithril_add_mod_curve25519, $div_with_rem\n");
}

// Table of startup routines required by Icarus Verilog
void (*vlog_startup_routines[])(void) = {
    register_mithril_vpi_tasks,
    0
};
