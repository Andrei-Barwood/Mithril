//
//  template_generator.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 20-08-25.
//

#ifndef MITHRIL_TEMPLATE_GENERATOR_H
#define MITHRIL_TEMPLATE_GENERATOR_H

#include "operations_config.h"

// Generate all operation files at once
int mithril_generate_all_files(const char* project_path);

// Generate specific operation files
int mithril_generate_specific_files(const char* project_path, const char** operation_names, int count);

// Generate single operation
int mithril_generate_single_operation(const char* project_path, const MithrilOperationConfig* config);

// Template generation functions
char* mithril_generate_header_template(const MithrilOperationConfig* config);
char* mithril_generate_source_template(const MithrilOperationConfig* config);

#endif
