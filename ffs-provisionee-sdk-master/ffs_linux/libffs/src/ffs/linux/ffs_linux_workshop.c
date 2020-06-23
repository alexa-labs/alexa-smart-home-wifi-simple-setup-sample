
/** 
 * Copyright 2020 Amazon.com, Inc. and its affiliates. All Rights Reserved.
* SPDX-License-Identifier: LicenseRef-.amazon.com.-AmznSL-1.0
*
* Licensed under the Amazon Software License (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
*  http://aws.amazon.com/asl/
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "ffs/linux/aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "ffs/common/ffs_check_result.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_wifi_manager.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_task.h"
#include "ffs/linux/ffs_linux_crypto_common.h"
#include "ffs/common/ffs_base64.h"
#include "ffs/linux/ffs_third_party_server_client.h"

#include <getopt.h>
#include <curl/curl.h>
#include <unistd.h>

#define HOST_ADDRESS_SIZE 255
#define ALEXA_ENDPOINT_SIZE                   25
#define SESSION_TOKEN_SIZE                    70
#define SESSION_TOKEN_DER_SIGNATURE_SIZE      72
#define SESSION_TOKEN_SIGNATURE_BASE64_SIZE   99
#define DEFAULT_WORKSHOP_DEVICE_IDENTIFIER   "device-id-123"
#define DEFAULT_PROVISIONING_TOPIC   "FFSWorkshop/ProvisionedDeviceTopic"

static char* provisionURL;
/**
 * @brief Default cert location
 */
static char certDirectory[PATH_MAX + 1] = "certs";

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
static char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
static uint32_t port = AWS_IOT_MQTT_PORT;

/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
static uint32_t publishCount = 0;

static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                           IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("Subscribe callback");
    IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

static void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    IOT_WARN("MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    IOT_UNUSED(data);

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            IOT_WARN("Manual Reconnect Successful");
        } else {
            IOT_WARN("Manual Reconnect Failed - %d", rc);
        }
    }
}

static size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    FfsStream_t *outputStream = (FfsStream_t *) userp;
    FFS_CHECK_RESULT(ffsFlushStream(outputStream));
    FFS_CHECK_RESULT(ffsWriteStream((uint8_t *)contents, size * nmemb, outputStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, outputStream));
    return size * nmemb;
}
static FFS_RESULT ffsCallDeviceProvision(char *requestData, bool needVerbose, bool needResponseData, FfsStream_t *responseStream) {
    CURL *curl;
    CURLcode res;

    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        ffsLogError("curl_global_init() failed: %s\n",
                    curl_easy_strerror(res));
        FFS_FAIL(FFS_ERROR);
    }
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, provisionURL);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(requestData));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData);
        if (needVerbose) {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }
        if (needResponseData) {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, responseStream);
        }
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            ffsLogError("curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return FFS_SUCCESS;
}


/**
 * @brief This method will be called to publish a message to MQ to reflect that the device is online now.
 */
static int ffsPublishToProvisioningQueue(struct FfsUserContext_s *userContext) {
    bool infinitePublishFlag = true;

    char rootCA[PATH_MAX + 1];
    char clientCRT[PATH_MAX + 1];
    char clientKey[PATH_MAX + 1];
    char CurrentWD[PATH_MAX + 1];
    char cPayload[2048];
    int32_t i = 0;
    IoT_Error_t rc = FAILURE;
    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
    //IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;
    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
    getcwd(CurrentWD, sizeof(CurrentWD));
    snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
    snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
    snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);
    IOT_DEBUG("rootCA %s", rootCA);
    IOT_DEBUG("clientCRT %s", clientCRT);
    IOT_DEBUG("clientKey %s", clientKey);
    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = HostAddress;
    mqttInitParams.port = port;
    mqttInitParams.pRootCALocation = rootCA;
    mqttInitParams.pDeviceCertLocation = clientCRT;
    mqttInitParams.pDevicePrivateKeyLocation = clientKey;
    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;
    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
    }
    connectParams.keepAliveIntervalInSec = 600;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
    connectParams.isWillMsgPresent = false;
    IOT_INFO("Connecting...");
    rc = aws_iot_mqtt_connect(&client, &connectParams);
    if(SUCCESS != rc) {
        IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return rc;
    }
    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
        return rc;
    }
    //sprintf(cPayload, "%s : %d ", "device_123 is online", 1);
// Prepare the json message for setup complete
    char setupCompleteRequestData[2048];
    char *setupCompleteRequestDataFormat = "{"
                                           "\"device\" : { "
                                           "\"sessionToken\" : \"%s\", "
                                           "\"signature\" : \"%s\", "
                                           "\"namingCategories\" : [\"LIGHT\"], "
                                           "\"id\" : \"%s\""
                                           " }"
                                           " }";
    // Read FFS SessionToken from configuration map.
    FFS_TEMPORARY_OUTPUT_STREAM(wssSessionTokenStream, SESSION_TOKEN_SIZE);
    FfsMapValue_t wssSessionTokenValue = {
            .type = FFS_MAP_VALUE_TYPE_STRING,
            .stringStream = wssSessionTokenStream
    };
    FFS_CHECK_RESULT(ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_DSS_SESSION_TOKEN, &wssSessionTokenValue));
    wssSessionTokenStream = wssSessionTokenValue.stringStream;
    // Sign the sessionToken with device private key.
    FFS_TEMPORARY_OUTPUT_STREAM(wssSessionTokenDerSignatureStream, SESSION_TOKEN_DER_SIGNATURE_SIZE);
    FFS_CHECK_RESULT(ffsSignPayload(userContext, &wssSessionTokenStream, &wssSessionTokenDerSignatureStream));
    // Base64 encode the sessionTokenSignature.
    FFS_TEMPORARY_OUTPUT_STREAM(wssSessionTokenBase64SignatureStream, SESSION_TOKEN_SIGNATURE_BASE64_SIZE);
    FFS_CHECK_RESULT(ffsEncodeBase64(&wssSessionTokenDerSignatureStream, 0, NULL, &wssSessionTokenBase64SignatureStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &wssSessionTokenBase64SignatureStream));
    char *sessionTokenSignatureString = (char *)FFS_STREAM_NEXT_READ(wssSessionTokenBase64SignatureStream);
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &wssSessionTokenStream));
    char *wssSessionToken = (char *)FFS_STREAM_NEXT_READ(wssSessionTokenStream);
    // Construct the request data with the request format.
    sprintf(setupCompleteRequestData, setupCompleteRequestDataFormat, wssSessionToken, sessionTokenSignatureString, DEFAULT_WORKSHOP_DEVICE_IDENTIFIER);
    ffsLogDebug(" request data : %s", setupCompleteRequestData);
//End preparing the message for setup complete
    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cPayload;
    paramsQOS1.isRetained = 0;
    sprintf(cPayload, setupCompleteRequestData);
    ffsLogDebug("Payload added");
    paramsQOS1.payloadLen = strlen(cPayload);
    rc = aws_iot_mqtt_publish(&client, DEFAULT_PROVISIONING_TOPIC, strlen(DEFAULT_PROVISIONING_TOPIC), &paramsQOS1);
    if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
        IOT_WARN("QOS0 publish ack not received.\n");
        rc = SUCCESS;
    }
    if (provisionURL){
        ffsLogDebug("Attempting to call provisioning URL : %s", provisionURL);
        FFS_CHECK_RESULT(ffsCallDeviceProvision(setupCompleteRequestData, true, false, NULL));
    }


    return rc;
}


/** Static function prototypes.
 */
static FFS_RESULT ffsParseCommandLine(struct FfsUserContext_s *userContext, int argc, char **argv);
static void ffsStartWifiScanCallback(struct FfsUserContext_s *userContext, FFS_RESULT result);
static FFS_RESULT ffsDeinitializeWifiManagerBlocking(struct FfsUserContext_s *userContext);
static void ffsDeinitializeWifiManagerCallback(struct FfsUserContext_s *userContext,
                                               FFS_RESULT result);

int main(int argc, char **argv)
{

    // Initialize the user context.
    FfsUserContext_t userContext;
    FFS_CHECK_RESULT(ffsInitializeUserContext(&userContext));
    srand(time(NULL));

    // Parse the command line arguments.
    FFS_CHECK_RESULT(ffsParseCommandLine(&userContext, argc, argv));

    // Initialize the Wi-Fi manager.
    FFS_CHECK_RESULT(ffsInitializeWifiManager(&userContext));

    // Start a background Wi-Fi scan.
    FFS_CHECK_RESULT(ffsWifiManagerStartScan(ffsStartWifiScanCallback));

    // Execute the Wi-Fi provisionee task.
    FFS_CHECK_RESULT(ffsWifiProvisioneeTask(&userContext));

    //ffsLogDebug( "Session ID - ");
    //ffsLogDebug( userContext.sessionIdBuffer);
    //try calling the mq function to make sure it's runNing and linking properly
    ffsLogDebug("Attempting to connect to AWS IOT MQ");
    int result = ffsPublishToProvisioningQueue(&userContext);
    if(SUCCESS == result) {
        ffsLogDebug("successfully sent provisioning message to mq " );
    } else{
        ffsLogDebug("failed to connect or send a provisioning message to mq");
    }
    // Deinitialize the Wi-Fi manager.
    FFS_CHECK_RESULT(ffsDeinitializeWifiManagerBlocking(&userContext));

    // Deinitialize the user context.
    FFS_CHECK_RESULT(ffsDeinitializeUserContext(&userContext));


    return 0;
}

/** @brief Parse command line arguments.
 */
static FFS_RESULT ffsParseCommandLine(struct FfsUserContext_s *userContext, int argc, char **argv) {

    const char *cloudPublicKeyPath = NULL;

    for (;;) {

        // Command-line options.
        static struct option options[] = {
                { "ssid", no_argument, 0, 's' },
                { "key", no_argument, 0, 'k' },
                { "host", no_argument, 0, 'h' },
                { "port", no_argument, 0, 'p' },
                { "cloud_public_key", no_argument, 0, 'c'},
                { "provisionUrl", no_argument, 0, 'u'},
                { NULL, 0, 0, 0 }
        };

        // getopt_long stores the option index here.
        int optionIndex = 0;

        int shortOption = getopt_long(argc, argv, "s:k:h:p:c:u:", options, &optionIndex);

        // Done with options?
        if (shortOption < 0) {
            break;
        }

        switch (shortOption) {
            case 's':
                ffsLogDebug("Use custom SSID \"%s\"", optarg);
                userContext->setupNetworkSsid = strdup(optarg);
                break;
            case 'k':
                ffsLogDebug("Use custom PSK", optarg);
                userContext->setupNetworkPsk = strdup(optarg);
                break;
            case 'h':
                ffsLogDebug("Use custom DSS host %s", optarg);
                userContext->dssHost = strdup(optarg);
                break;
            case 'p':
                ffsLogDebug("Use custom DSS port %s", optarg);
                userContext->dssPort = atoi(optarg);
                userContext->hasDssPort = true;
                break;
            case 'c':
                ffsLogDebug("Use custom cloud public key %s", optarg);
                cloudPublicKeyPath = strdup(optarg);
                break;
            case 'u':
                ffsLogDebug("Use provision url %s", optarg);
                provisionURL = strdup(optarg);
                break;
            default:
                ffsLogError("Unknown option %c", shortOption);
        }

    }

    FFS_CHECK_RESULT(ffsInitializePublicKey(userContext, cloudPublicKeyPath));

    return FFS_SUCCESS;
}

/** @brief Callback for the start background Wi-Fi scan call.
 */
static void ffsStartWifiScanCallback(struct FfsUserContext_s *userContext, FFS_RESULT result)
{
    (void) userContext;

    ffsLogDebug("Wi-Fi scan complete with result %s", ffsGetResultString(result));
}

/** @brief Block the main thread until the Wi-Fi manager is deinitialized.
 */
static FFS_RESULT ffsDeinitializeWifiManagerBlocking(struct FfsUserContext_s *userContext) {
    FFS_CHECK_RESULT(ffsDeinitializeWifiManager(userContext, ffsDeinitializeWifiManagerCallback));

    if (sem_wait(userContext->ffsTaskWifiSemaphore)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/** @brief Callback for the deinitialize Wi-Fi manager call.
 */
static void ffsDeinitializeWifiManagerCallback(struct FfsUserContext_s *userContext,
                                               FFS_RESULT result) {
    (void) userContext;

    ffsLogDebug("Deinitialize Wi-Fi manager complete with result %s", ffsGetResultString(result));

    if (sem_post(userContext->ffsTaskWifiSemaphore)) {
        ffsLogError("Failed to post to Wi-Fi semaphore");
    }
}
