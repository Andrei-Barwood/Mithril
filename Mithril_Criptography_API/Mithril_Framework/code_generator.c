//
//  code_generator.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 20-08-25.
//

#include "code_generator.h"
#include "template_generator.h"
#include <stdio.h>

int mithril_generate_all_required_files(const char* project_path) {
    printf("🚀 Generating all Mithril operation files...\n");
    return mithril_generate_all_files(project_path);
}

int mithril_generate_specific_operations(const char* project_path, const char** operation_names, int count) {
    printf("🎯 Generating specific operations...\n");
    return mithril_generate_specific_files(project_path, operation_names, count);
}

int mithril_add_new_operation(const char* project_path,
                             const char* operation_name,
                             const char* function_name,
                             const char* file_name,
                             const char* description,
                             const char* framework_function) {
    
    MithrilOperationConfig new_config = {
        .operation_name = operation_name,
        .function_name = function_name,
        .file_name = file_name,
        .description = description,
        .framework_function = framework_function
    };
    
    return mithril_generate_single_operation(project_path, &new_config);
}
