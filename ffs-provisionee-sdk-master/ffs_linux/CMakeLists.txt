cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(FrustrationFreeSetupLinux)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# 0 warnings.
set_property(DIRECTORY . APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra)

# Find any local CMake configuration file and add the directory to the search path.
function(find_package_configuration PACKAGE_NAME)

    # Find possible package configuration files.
    file(GLOB_RECURSE PACKAGE_CONFIG_FILES
            ../*/${PACKAGE_NAME}Config.cmake
            )

    # Add each configuration file directory to the search path.
    foreach(CONFIG_FILE IN LISTS PACKAGE_CONFIG_FILES)
        if (NOT ${CONFIG_FILE} MATCHES "Export")
            message(STATUS "Found ${PACKAGE_NAME} configuration file: ${CONFIG_FILE}")
            get_filename_component(CONFIG_FILE_DIRECTORY ${CONFIG_FILE} DIRECTORY)
            set(CMAKE_PREFIX_PATH ${CONFIG_FILE_DIRECTORY};${CMAKE_PREFIX_PATH} PARENT_SCOPE)
        endif()
    endforeach(CONFIG_FILE)

endfunction(find_package_configuration)

# FFS SDK (use the development package if it exists).
find_package_configuration(FrustrationFreeSetup)
find_package(FrustrationFreeSetup REQUIRED)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE FFS_WIFI_PROVISIONEE_LINUX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/libffs/src/*.c
        )

# Exclude the demo main file.
FOREACH(item ${FFS_WIFI_PROVISIONEE_LINUX_SOURCES})
    IF(${item} MATCHES ${CMAKE_CURRENT_SOURCE_DIR}/libffs/src/ffs/linux/ffs_linux_main.c)
        LIST(REMOVE_ITEM FFS_WIFI_PROVISIONEE_LINUX_SOURCES ${item})
    ENDIF(${item} MATCHES ${CMAKE_CURRENT_SOURCE_DIR}/libffs/src/ffs/linux/ffs_linux_main.c)
ENDFOREACH(item)


## Begin AWS IOT BUILD LIBs

include_directories(${CMAKE_SOURCE_DIR}/awsiot/external_libs/jsmn)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/configs)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/certs)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/include)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/include/mbedtls)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/include)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/platform/linux/common)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/platform/linux/mbedtls)
include_directories(${CMAKE_SOURCE_DIR}/awsiot/platform/linux/pthread)

add_library(aws_iot_embedded
        ${CMAKE_SOURCE_DIR}/awsiot/external_libs/jsmn/jsmn.c
        ${CMAKE_SOURCE_DIR}/awsiot/platform/linux/common/timer.c
        ${CMAKE_SOURCE_DIR}/awsiot/platform/linux/mbedtls/network_mbedtls_wrapper.c
        ${CMAKE_SOURCE_DIR}/awsiot/platform/linux/pthread/threads_pthread_wrapper.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_jobs_interface.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_jobs_json.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_jobs_topics.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_jobs_types.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_json_utils.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_common_internal.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_connect.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_publish.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_subscribe.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_unsubscribe.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_mqtt_client_yield.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_shadow.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_shadow_actions.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_shadow_json.c
        ${CMAKE_SOURCE_DIR}/awsiot/src/aws_iot_shadow_records.c
        )

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/library)
#LINK_DIRECTORIES(external_libs/mbedTLS/crypto/library)

TARGET_LINK_LIBRARIES(aws_iot_embedded
        ${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/library/libmbedtls.a
        ${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/library/libmbedcrypto.a
        ${CMAKE_SOURCE_DIR}/awsiot/external_libs/mbedTLS/library/libmbedx509.a)


#add_executable(aws_iot_embedded)
add_executable(subscribe_publish_sample
        ${CMAKE_SOURCE_DIR}/awsiot/samples/linux/subscribe_publish_sample/aws_iot_config.h
        ${CMAKE_SOURCE_DIR}/awsiot/samples/linux/subscribe_publish_sample/subscribe_publish_sample.c)


target_link_libraries(subscribe_publish_sample PUBLIC
        aws_iot_embedded)

#END  AWS IOT BUILD LIBs



add_library(FrustrationFreeSetupLinux
        ${FFS_WIFI_PROVISIONEE_LINUX_SOURCES}
        )

add_executable(FrustrationFreeSetupLinuxDemo
        ${CMAKE_CURRENT_SOURCE_DIR}/libffs/src/ffs/linux/ffs_linux_main.c
        )
add_executable(FrustrationFreeSetupLinuxWorkshop
        ${CMAKE_CURRENT_SOURCE_DIR}/libffs/src/ffs/linux/ffs_linux_workshop.c
        )
target_include_directories(FrustrationFreeSetupLinux PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libffs/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(FrustrationFreeSetupLinux PUBLIC
        FrustrationFreeSetup
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        OpenSSL::Crypto
        )

target_link_libraries(FrustrationFreeSetupLinuxDemo PUBLIC
        aws_iot_embedded
        FrustrationFreeSetup
        FrustrationFreeSetupLinux
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        OpenSSL::Crypto
        )

target_link_libraries(FrustrationFreeSetupLinuxWorkshop PUBLIC
        aws_iot_embedded
        FrustrationFreeSetup
        FrustrationFreeSetupLinux
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        OpenSSL::Crypto
        )

# Debug.
option(ENABLE_DEBUG "Enable debug" ON)
if (${ENABLE_DEBUG})
    message("FFS - Enable Wi-Fi provisionee Linux debug")

    target_compile_definitions(FrustrationFreeSetupLinux PUBLIC
            -DFFS_DEBUG
            -DFFS_LINUX_LOGGING_STDOUT
            )
endif()

# Testing.
option(ENABLE_TESTS "Enable tests" ON)
if (${ENABLE_TESTS})
    message("FFS - Enable Wi-Fi provisionee Linux tests")

    if(NOT DEFINED BRAZIL_TESTING)
        set(BRAZIL_TESTING ON)
    endif()

    enable_testing()

    include(libffs/test/LocalCoverage.cmake)
    add_subdirectory(libffs/test)

    # Currently a null test to make Brazil happy.
    enable_testing()
    add_test(NAME null_test COMMAND echo "no tests")
endif()

# Need the full executable path on Macs. Note: Copy c_rehash to /usr/local/bin from an openssl install.
if(APPLE)
    execute_process(COMMAND /bin/cp -r ${CMAKE_SOURCE_DIR}/libffs/data ${CMAKE_CURRENT_BINARY_DIR})
    execute_process(COMMAND /bin/cp -r ${CMAKE_SOURCE_DIR}/awsiot/certs ${CMAKE_CURRENT_BINARY_DIR})
    execute_process(COMMAND /usr/local/bin/c_rehash ${CMAKE_CURRENT_BINARY_DIR}/data/dss_certificates)
else()
    execute_process(COMMAND cp -r ${CMAKE_SOURCE_DIR}/libffs/data ${CMAKE_CURRENT_BINARY_DIR})
    execute_process(COMMAND cp -r ${CMAKE_SOURCE_DIR}/awsiot/certs ${CMAKE_CURRENT_BINARY_DIR})
    execute_process(COMMAND c_rehash ${CMAKE_CURRENT_BINARY_DIR}/data/dss_certificates)
endif()

install(TARGETS FrustrationFreeSetupLinuxDemo
        RUNTIME  DESTINATION bin)  # This is for Windows
