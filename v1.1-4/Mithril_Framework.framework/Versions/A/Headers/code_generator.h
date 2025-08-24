//
//  code_generator.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 20-08-25.
//

#ifndef MITHRIL_CODE_GENERATOR_H
#define MITHRIL_CODE_GENERATOR_H

// Generate all operations
int mithril_generate_all_required_files(const char* project_path);

// Generate specific operations
int mithril_generate_specific_operations(const char* project_path, const char** operation_names, int count);

// Add new operation dynamically
int mithril_add_new_operation(const char* project_path,
                             const char* operation_name,
                             const char* function_name,
                             const char* file_name,
                             const char* description,
                             const char* framework_function);

#endif
