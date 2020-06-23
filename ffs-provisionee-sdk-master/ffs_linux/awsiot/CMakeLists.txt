cmake_minimum_required(VERSION 3.15)
project(aws_iot_embedded)
set_property(DIRECTORY . APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-unused-variable)
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS ON)
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)


include_directories(${CMAKE_SOURCE_DIR}/awsiot/awsiot/external_libs/jsmn)
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
add_executable(subscribe_publish
        ${CMAKE_SOURCE_DIR}/awsiot/samples/linux/subscribe_publish_sample/aws_iot_config.h
        ${CMAKE_SOURCE_DIR}/awsiot/samples/linux/subscribe_publish_sample/subscribe_publish_sample.c)


target_link_libraries(subscribe_publish PUBLIC
        aws_iot_embedded)




export(TARGETS aws_iot_embedded FILE aws_iot_embeddedConfig.cmake)
install(TARGETS aws_iot_embedded EXPORT aws_iot_embeddedConfig
        ARCHIVE  DESTINATION lib
        LIBRARY  DESTINATION lib
        RUNTIME  DESTINATION bin)
install(EXPORT aws_iot_embeddedConfig DESTINATION cmake)
#install(DIRECTORY libffs/include/ DESTINATION include)
