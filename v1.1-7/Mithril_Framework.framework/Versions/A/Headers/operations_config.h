//
//  operations_config.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 20-08-25.
//

#ifndef MITHRIL_OPERATIONS_CONFIG_H
#define MITHRIL_OPERATIONS_CONFIG_H

typedef struct {
    const char* operation_name;      // "addition", "subtraction"
    const char* function_name;       // "mithril_add", "mithril_subtract"
    const char* file_name;          // "mithril_addition", "mithril_subtraction"
    const char* description;        // "Performs addition of two fmpz_t numbers"
    const char* framework_function; // "add_l", "sub_l"
} MithrilOperationConfig;

// Get all available operations
const MithrilOperationConfig* mithril_get_all_operations();
int mithril_get_operations_count();

#endif
