project(astarte-device-sdk-qt5 CXX)

cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#################################################################################################
# Modify these variables to what suits the application best. Remember, no dashes in the version!
set(ASTARTE_DEVICE_SDK_QT5_ABI_VERSION 0)
set(ASTARTE_DEVICE_SDK_QT5_MINOR_VERSION 91)
set(ASTARTE_DEVICE_SDK_QT5_RELEASE_VERSION 112)
option(ASTARTE_DEVICE_SDK_QT5_DEVELOPMENT_RELEASE "Must be ON unless we're releasing" ON)

option(ENABLE_WERROR "Enables WError. Always enable when developing, and disable when releasing." OFF)

option(ENABLE_ASTARTE_DEVICE_SDK_QT5_TEST_CODEPATHS "Enable specific codepaths needed for autotests. As they pose a potential security threat, disable when building a release build." ON)

#################################################################################################

set(ASTARTE_DEVICE_SDK_QT5_VERSION ${ASTARTE_DEVICE_SDK_QT5_ABI_VERSION}.${ASTARTE_DEVICE_SDK_QT5_MINOR_VERSION}.${ASTARTE_DEVICE_SDK_QT5_RELEASE_VERSION})
if (ASTARTE_DEVICE_SDK_QT5_DEVELOPMENT_RELEASE)
    set(ASTARTE_DEVICE_SDK_QT5_VERSION_STRING ${ASTARTE_DEVICE_SDK_QT5_ABI_VERSION}.${ASTARTE_DEVICE_SDK_QT5_MINOR_VERSION}.${ASTARTE_DEVICE_SDK_QT5_RELEASE_VERSION}+git)

    # Enable debug messages
    add_definitions(-DDEBUG_MESSAGES_DEFAULT_LEVEL=QtDebugMsg)
else (ASTARTE_DEVICE_SDK_QT5_DEVELOPMENT_RELEASE)
    set(ASTARTE_DEVICE_SDK_QT5_VERSION_STRING ${ASTARTE_DEVICE_SDK_QT5_ABI_VERSION}.${ASTARTE_DEVICE_SDK_QT5_MINOR_VERSION}.${ASTARTE_DEVICE_SDK_QT5_RELEASE_VERSION})

    # Show only warning messages and worst
    add_definitions(-DDEBUG_MESSAGES_DEFAULT_LEVEL=QtWarningMsg)
endif (ASTARTE_DEVICE_SDK_QT5_DEVELOPMENT_RELEASE)

find_package(Qt5 COMPONENTS Core Concurrent Network Test Sql REQUIRED)
# We need OpenSSL for building the transport library
find_package(OpenSSL REQUIRED)
# We need MQTT Client (mosquittopp) for building the transport library
#pkg_check_modules(MOSQUITTO libmosquittopp REQUIRED)


include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DASTARTE_DEVICE_SDK_QT5_VERSION="${ASTARTE_DEVICE_SDK_QT5_VERSION_STRING}")

if (ENABLE_ASTARTE_DEVICE_SDK_QT5_TEST_CODEPATHS)
    add_definitions(-DENABLE_TEST_CODEPATHS)
endif (ENABLE_ASTARTE_DEVICE_SDK_QT5_TEST_CODEPATHS)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

add_definitions("-std=gnu++11")

# We want explicit literals all over the place, and fast concat
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_USE_QSTRINGBUILDER)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_DATA_DIR share/hemera/sdk CACHE PATH "Installation directory for data files of the SDK")
set(INSTALL_INTERFACES_DIR share/hyperspace/interfaces CACHE PATH "Installation directory for Hyperspace Interfaces defined by applications.")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_PLUGINS_DIR lib/hyperspace/plugins CACHE PATH "Installation directory for plugins")
set(INSTALL_PKGCONFIG_DIR ${INSTALL_LIB_DIR}/pkgconfig CACHE PATH "Installation directory for pkgconfig files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN DATA INCLUDE CMAKE INTERFACES QML_PLUGINS)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# Enable compilation of tests and examples by default only if we're not in a submodule
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(ENABLE_ASTARTE_DEVICE_SDK_QT5_EXAMPLES "Enable compilation of examples" ON)
    option(ENABLE_ASTARTE_DEVICE_SDK_QT5_TESTS "Enable compilation of tests" ON)
else (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(ENABLE_ASTARTE_DEVICE_SDK_QT5_EXAMPLES "Enable compilation of examples" OFF)
    option(ENABLE_ASTARTE_DEVICE_SDK_QT5_TESTS "Enable compilation of tests" OFF)
endif (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

option(ENABLE_ASTARTE_DEVICE_SDK_QT5_COVERAGE "Enable compiler coverage" OFF)

# Definitions
add_definitions(-DASTARTE_DEVICE_SDK_QT5_VERSION="${ASTARTE_DEVICE_SDK_QT5_VERSION_STRING}")

# Create paths for the AstarteDeviceSDKQt5Config.cmake and AstarteDeviceSDKQt5ConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}/AstarteDeviceSDKQt5" "${INSTALL_INCLUDE_DIR}")

set(PROJECT_VERSION_FULL ${ASTARTE_DEVICE_SDK_QT5_VERSION_STRING})

include_directories(
    ${CMAKE_SOURCE_DIR}/astarte-device-sdk
    ${CMAKE_SOURCE_DIR}/astarte-transport
    ${CMAKE_SOURCE_DIR}/astarte-utils
    ${CMAKE_SOURCE_DIR}/HemeraCore
    ${CMAKE_SOURCE_DIR}/hyperdrive
    ${CMAKE_SOURCE_DIR}/hyperspace
    ${CMAKE_SOURCE_DIR}/transports
)

# sources
set(astartedevicesdk_SRCS
    astarte-device-sdk/AstarteDeviceSDK.cpp

    astarte-transport/astartetransport.cpp
    astarte-transport/astartetransportcache.cpp

    astarte-utils/AstarteGenericConsumer.cpp
    astarte-utils/AstarteGenericProducer.cpp
    astarte-utils/QJsonSchemaChecker.cpp
    astarte-utils/ValidateInterfaceOperation.cpp

    HemeraCore/hemeraoperation.cpp
    HemeraCore/hemeracommonoperations.cpp
    HemeraCore/hemeraasyncinitobject.cpp
    HemeraCore/hemerafingerprints.cpp
    HemeraCore/hemerafakehardwareidoperation.cpp

    hyperdrive/cachemessage.cpp
    hyperdrive/hyperdriveinterface.cpp
    hyperdrive/hyperdriveutils.cpp

    hyperspace/BSONDocument.cpp
    hyperspace/BSONSerializer.cpp
    hyperspace/Fluctuation.cpp
    hyperspace/Rebound.cpp
    hyperspace/Wave.cpp
    hyperspace/AbstractWaveTarget.cpp
    hyperspace/ConsumerAbstractAdaptor.cpp
    hyperspace/ProducerAbstractInterface.cpp

    transports/transportdatabasemanager.cpp
    transports/hyperdrivemqttclientwrapper.cpp
    transports/astartecrypto.cpp
    transports/astarteendpoint.cpp
    transports/astarteverifycertificateoperation.cpp
    transports/astartehttpendpoint.cpp
    transports/astartepairoperation.cpp
)

set(astartedevicesdk_HDRS
    astarte-device-sdk/AstarteDeviceSDK.h

    astarte-transport/astartetransport.h
    astarte-transport/astartetransportcache.h

    astarte-utils/AstarteGenericConsumer.h
    astarte-utils/AstarteGenericProducer.h
    astarte-utils/QJsonSchemaChecker.h
    astarte-utils/ValidateInterfaceOperation.h

    HemeraCore/hemeraoperation.h
    HemeraCore/hemeracommonoperations.h
    HemeraCore/hemeraasyncinitobject.h
    HemeraCore/hemerafingerprints.h
    HemeraCore/hemerafakehardwareidoperation.h
    HemeraCore/hemeraglobal.h
    HemeraCore/hemeraliterals.h

    hyperdrive/cachemessage.h
    hyperdrive/hyperdriveinterface.h
    hyperdrive/hyperdriveprotocol.h
    hyperdrive/hyperdriveutils.h

    hyperspace/BSONDocument.h
    hyperspace/BSONSerializer.h
    hyperspace/Fluctuation.h
    hyperspace/Global.h
    hyperspace/Rebound.h
    hyperspace/Wave.h
    hyperspace/AbstractWaveTarget.h
    hyperspace/ConsumerAbstractAdaptor.h
    hyperspace/ProducerAbstractInterface.h

    transports/transportdatabasemanager.h
    transports/hyperdrivemqttclientwrapper.h
    transports/astartecrypto.h
    transports/astarteendpoint.h
    transports/astarteverifycertificateoperation.h
    transports/astartehttpendpoint.h
)

add_library(AstarteDeviceSDKQt5 SHARED ${astartedevicesdk_SRCS})

set_target_properties(AstarteDeviceSDKQt5 PROPERTIES
                      OUTPUT_NAME AstarteDeviceSDKQt5
                      SOVERSION ${ASTARTE_DEVICE_SDK_QT5_ABI_VERSION}
                      VERSION ${ASTARTE_DEVICE_SDK_QT5_VERSION}
                      PUBLIC_HEADER "${astartedevicesdk_HDRS}")

target_link_libraries(AstarteDeviceSDKQt5
                      Qt5::Core Qt5::Network Qt5::Sql mosquittopp
                      ${OPENSSL_LIBRARIES})

# Install phase
install(TARGETS AstarteDeviceSDKQt5
        EXPORT  AstarteDeviceSDKQt5Targets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/AstarteDeviceSDKQt5"
        COMPONENT AstarteDeviceSDKQt5)

# Headers
install(FILES
    HemeraCore/HemeraCore/AsyncInitObject
    HemeraCore/HemeraCore/CommonOperations
    HemeraCore/HemeraCore/Fingerprints
    HemeraCore/HemeraCore/Global
    HemeraCore/HemeraCore/Literals
    HemeraCore/HemeraCore/Operation
    DESTINATION ${INSTALL_INCLUDE_DIR}/AstarteDeviceSDKQt5/HemeraCore)

install(FILES
    hyperspace/HyperspaceCore/AbstractWaveTarget
    hyperspace/HyperspaceCore/BSONDocument
    hyperspace/HyperspaceCore/BSONSerializer
    hyperspace/HyperspaceCore/Fluctuation
    hyperspace/HyperspaceCore/Global
    hyperspace/HyperspaceCore/Rebound
    hyperspace/HyperspaceCore/Wave
    DESTINATION ${INSTALL_INCLUDE_DIR}/AstarteDeviceSDKQt5/HyperspaceCore)

install(FILES
    hyperspace/HyperspaceProducerConsumer/ConsumerAbstractAdaptor
    hyperspace/HyperspaceProducerConsumer/ProducerAbstractInterface
    DESTINATION ${INSTALL_INCLUDE_DIR}/AstarteDeviceSDKQt5/HyperspaceProducerConsumer)


# Export target
install(EXPORT AstarteDeviceSDKQt5Targets DESTINATION ${INSTALL_CMAKE_DIR}/AstarteDeviceSDKQt5)

set(ASTARTE_DEVICE_SDK_QT5_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/AstarteDeviceSDKQt5")

# Pkgconfig
configure_file(AstarteDeviceSDKQt5.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/AstarteDeviceSDKQt5.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/AstarteDeviceSDKQt5.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR} COMPONENT AstarteDeviceSDKQt5)

# Files
install(FILES astarte-device-sdk/interface.json
        DESTINATION /usr/share/hyperdrive/transport-astarte COMPONENT AstarteDeviceSDKQt5)
# Files
install(FILES astarte-transport/db/migrations/001_create_cachemessages.sql astarte-transport/db/migrations/002_create_persistent_entries.sql
        DESTINATION /usr/share/hyperdrive/transport-astarte/db/migrations COMPONENT AstarteDeviceSDKQt5)

## Examples
set(astartedevicesdkexample_SRCS
    astarte-utils/astarte-validate-interface/astarte-validate-interface.cpp
    astarte-utils/ValidateInterfaceOperation.cpp
)

add_executable(astarte-validate-interface ${astartedevicesdkexample_SRCS})

target_link_libraries(astarte-validate-interface AstarteDeviceSDKQt5
                      Qt5::Core mosquittopp mosquitto
                      ${OPENSSL_LIBRARIES})

install(TARGETS astarte-validate-interface
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        COMPONENT AstarteDeviceSDKQt5)

configure_file(AstarteDeviceSDKQt5Config.cmake.in
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/AstarteDeviceSDKQt5Config.cmake" @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/BasicFindPackageVersion.cmake.in
  "${CMAKE_BINARY_DIR}/AstarteDeviceSDKQt5ConfigVersion.cmake" @ONLY)

# Install the AstarteDeviceSDKQt5Config.cmake and AstarteDeviceSDKQt5ConfigVersion.cmake
install(FILES
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/AstarteDeviceSDKQt5Config.cmake"
  "${CMAKE_BINARY_DIR}/AstarteDeviceSDKQt5ConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}/AstarteDeviceSDKQt5" COMPONENT AstarteDeviceSDKQt5)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
