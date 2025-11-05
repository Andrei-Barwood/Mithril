# examples/basic/CMakeLists.txt

find_package(Boost 1.82 REQUIRED COMPONENTS system)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

# Servidor
add_executable(accepting_connection_sodium 
    accepting_connection_sodium.cpp
)
target_compile_features(accepting_connection_sodium PRIVATE cxx_std_20)
target_link_libraries(accepting_connection_sodium 
    PRIVATE 
    Boost::system 
    ${SODIUM_LIBRARIES}
)

# Cliente
add_executable(secure_client 
    secure_client.cpp
)
target_compile_features(secure_client PRIVATE cxx_std_20)
target_link_libraries(secure_client 
    PRIVATE 
    Boost::system 
    ${SODIUM_LIBRARIES}
)
