# ============================================================================
# Mithril Examples - Tidal Energy Generation Network
# File: examples/tidal_energy/CMakeLists.txt
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(MithrilTidalExamples LANGUAGES CXX)

function(mithril_configure_tidal_example target_name source_file)
    add_executable(${target_name} ${source_file})
    target_compile_features(${target_name} PRIVATE cxx_std_20)
    target_link_libraries(${target_name}
        PRIVATE
            mithril_network
            mithril_v2
            Threads::Threads
    )

    if(APPLE)
        target_link_libraries(${target_name} PRIVATE "-framework Security")
    endif()

    if(MITHRIL_ENABLE_WARNINGS)
        mithril_set_compiler_warnings(${target_name} IS_C_ONLY FALSE ENABLE_WERROR ${MITHRIL_ENABLE_WERROR})
    endif()
endfunction()

mithril_configure_tidal_example(tidal_sensor_server tidal_sensor_server.cpp)
mithril_configure_tidal_example(control_panel_client control_panel_client.cpp)
mithril_configure_tidal_example(data_aggregator data_aggregator.cpp)
