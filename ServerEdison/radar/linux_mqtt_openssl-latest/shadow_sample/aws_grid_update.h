#ifndef SERVER_GRID_AWS_MQTT_H
#define SERVER_GRID_AWS_MQTT_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <limits.h>

#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_shadow_json_data.h"
#include "aws_iot_config.h"
#include "aws_iot_mqtt_interface.h"

#define ROOMTEMPERATURE_UPPERLIMIT 32.0f
#define ROOMTEMPERATURE_LOWERLIMIT 25.0f
#define STARTING_ROOMTEMPERATURE ROOMTEMPERATURE_LOWERLIMIT

static void simulateRoomTemperature(float *pRoomTemperature) {
	static float deltaChange;

	if (*pRoomTemperature >= ROOMTEMPERATURE_UPPERLIMIT) {
		deltaChange = -0.5f;
	} else if (*pRoomTemperature <= ROOMTEMPERATURE_LOWERLIMIT) {
		deltaChange = 0.5f;
	}

	*pRoomTemperature += deltaChange;
}

void ShadowUpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
		const char *pReceivedJsonDocument, void *pContextData) {

	if (status == SHADOW_ACK_TIMEOUT) {
		INFO("Update Timeout--");
	} else if (status == SHADOW_ACK_REJECTED) {
		INFO("Update RejectedXX");
	} else if (status == SHADOW_ACK_ACCEPTED) {
		INFO("Update Accepted !!");
	}
}

void windowActuate_Callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext) {
	if (pContext != NULL) {
		INFO("Delta - Window state changed to %d", *(bool *)(pContext->pData));
	}
}

char certDirectory[PATH_MAX + 1] = "../certs";
char HostAddress[255] = AWS_IOT_MQTT_HOST;
uint32_t port = AWS_IOT_MQTT_PORT;
uint8_t numPubs = 5;

// void parseInputArgsForConnectParams(int argc, char** argv) {
// 	int opt;

// 	while (-1 != (opt = getopt(argc, argv, "h:p:c:n:"))) {
// 		switch (opt) {
// 		case 'h':
// 			strcpy(HostAddress, optarg);
// 			DEBUG("Host %s", optarg);
// 			break;
// 		case 'p':
// 			port = atoi(optarg);
// 			DEBUG("arg %s", optarg);
// 			break;
// 		case 'c':
// 			strcpy(certDirectory, optarg);
// 			DEBUG("cert root directory %s", optarg);
// 			break;
// 		case 'n':
// 			numPubs = atoi(optarg);
// 			DEBUG("num pubs %s", optarg);
// 			break;
// 		case '?':
// 			if (optopt == 'c') {
// 				ERROR("Option -%c requires an argument.", optopt);
// 			} else if (isprint(optopt)) {
// 				WARN("Unknown option `-%c'.", optopt);
// 			} else {
// 				WARN("Unknown option character `\\x%x'.", optopt);
// 			}
// 			break;
// 		default:
// 			ERROR("ERROR in command line argument parsing");
// 			break;
// 		}
// 	}

// }

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

IoT_Error_t rc = NONE_ERROR;

MQTTClient_t mqttClient;

jsonStruct_t grid_tracker;

char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);

int thing_shadow_setup() {
	
	int32_t i = 0;
	
	aws_iot_mqtt_init(&mqttClient);

	char *pJsonStringToUpdate;
	float temperature = 0.0;

	//JSON Struct for shadow information
	bool windowOpen = false;
	grid_tracker.cb = windowActuate_Callback;
	grid_tracker.pData = &windowOpen;
	grid_tracker.pKey = "active_tiles";
	grid_tracker.type = SHADOW_JSON_BOOL;


	//Authentication variables
	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];
	char cafileName[] = AWS_IOT_ROOT_CA_FILENAME;
	char clientCRTName[] = AWS_IOT_CERTIFICATE_FILENAME;
	char clientKeyName[] = AWS_IOT_PRIVATE_KEY_FILENAME;

	//parseInputArgsForConnectParams(argc, argv);

	INFO("\nAWS IoT SDK Version(dev) %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	getcwd(CurrentWD, sizeof(CurrentWD));
	sprintf(rootCA, "%s/%s/%s", CurrentWD, certDirectory, cafileName);
	sprintf(clientCRT, "%s/%s/%s", CurrentWD, certDirectory, clientCRTName);
	sprintf(clientKey, "%s/%s/%s", CurrentWD, certDirectory, clientKeyName);

	DEBUG("Using rootCA %s", rootCA);
	DEBUG("Using clientCRT %s", clientCRT);
	DEBUG("Using clientKey %s", clientKey);

	//Initializing Thing Shadow parameters
	ShadowParameters_t sp = ShadowParametersDefault;
	sp.pMyThingName = AWS_IOT_MY_THING_NAME;
	sp.pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
	sp.pHost = HostAddress;
	sp.port = port;
	sp.pClientCRT = clientCRT;
	sp.pClientKey = clientKey;
	sp.pRootCA = rootCA;

	INFO("Shadow Init");
	rc = aws_iot_shadow_init(&mqttClient);

	INFO("Shadow Connect");
	rc = aws_iot_shadow_connect(&mqttClient, &sp);

	if (NONE_ERROR != rc) {
		ERROR("Shadow Connection Error %d", rc);
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = mqttClient.setAutoReconnectStatus(true);
	if (NONE_ERROR != rc) {
		ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	rc = aws_iot_shadow_register_delta(&mqttClient, &grid_tracker);

	if (NONE_ERROR != rc) {
		ERROR("Shadow Register Delta Error");
	}
	temperature = STARTING_ROOMTEMPERATURE;
	return rc;
}

int report_to_thing_shadow(jsonStruct_t grid_tracker) {
	//Publish any change to the AWS cloud
	if (NETWORK_ATTEMPTING_RECONNECT == rc || RECONNECT_SUCCESSFUL == rc || NONE_ERROR == rc) {
		rc = aws_iot_shadow_yield(&mqttClient, 200);
		if (NETWORK_ATTEMPTING_RECONNECT == rc) {
			sleep(1);
		}
		INFO("\n=======================================================================================\n");
		//INFO("On Device: window state %s", windowOpen?"true":"false");
		//simulateRoomTemperature(&temperature);

		rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
		if (rc == NONE_ERROR) {
			rc = aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 1, &grid_tracker);
			if (rc == NONE_ERROR) {
				rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
				if (rc == NONE_ERROR) {
					INFO("Update Shadow: %s", JsonDocumentBuffer);
					rc = aws_iot_shadow_update(&mqttClient, AWS_IOT_MY_THING_NAME, JsonDocumentBuffer,
							ShadowUpdateStatusCallback, NULL, 4, true);
				}
			}
		}
		INFO("*****************************************************************************************\n");
	}

	if (NONE_ERROR != rc) {
		ERROR("An error occurred %d", rc);
		INFO("Disconnecting");
		rc = aws_iot_shadow_disconnect(&mqttClient);
		if (NONE_ERROR != rc) {
			ERROR("Disconnect error %d", rc);
		}
	}
	return rc;
}

#endif
