include_directories(${CMAKE_BINARY_DIR}/astarte-utils
                    ${CMAKE_SOURCE_DIR}/astarte-utils
)

set(astarte-interface_SRCS
    astarte-interface.cpp
    ValidateInterfaceOperation.cpp
)

# final libraries
add_executable(astarte-interface ${astarte-interface_SRCS})
target_link_libraries(astarte-interface Qt5::Core astarte-utils)

if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    target_link_libraries(astarte-interface gcov)
endif ()

# Install phase
install(TARGETS astarte-interface
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT shlib
        COMPONENT Tools)

