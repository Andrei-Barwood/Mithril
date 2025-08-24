//
//  operations_config.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 20-08-25.
//

#include "operations_config.h"

// Define all your operations here
static const MithrilOperationConfig operations[] = {
    {
        .operation_name = "addition",
        .function_name = "add_l",
        .file_name = "02_addition.c",
        .description = "",
        .framework_function = {"fmpz_add_modern", "fmpz_add_with_legacy_overflow_check"}
    },
    {
        .operation_name = "subtraction",
        .function_name = "sub_l",
        .file_name = "04_subtraction.c",
        .description = "",
        .framework_function = "sub_l"  // Assuming you have this function
    },
    
    {
        .operation_name = "mixed addtion",
        .function_name = "uadd_l",
        .file_name = "06_mixed_addition.c",
        .description = "",
        .framework_function = "uadd_l"  // Assuming you have this function
    },
    
    {
        .operation_name = "mixed subtraction",
        .function_name = "usub_l",
        .file_name = "08_mixed_subtraction.c",
        .description = "",
        .framework_function = "usub_l"  // Assuming you have this function
    },

    {
        .operation_name = "increment",
        .function_name = "inc_l",
        .file_name = "10_increment.c",
        .description = "",
        .framework_function = "inc_l"  // Assuming you have this function
    },

    {
        .operation_name = "decrement",
        .function_name = "mithril_subtract",
        .file_name = "12_decrement.c",
        .description = "",
        .framework_function = "dec_l"  // Assuming you have this function
    },

    
    {
        .operation_name = "multiplication",
        .function_name = "mithril_multiply",
        .file_name = "mithril_multiplication",
        .description = "",
        .framework_function = "umul_l"  // Assuming you have this function
    }
    // Add more operations here easily!
};

const MithrilOperationConfig* mithril_get_all_operations() {
    return operations;
}

int mithril_get_operations_count() {
    return sizeof(operations) / sizeof(operations[0]);
}
