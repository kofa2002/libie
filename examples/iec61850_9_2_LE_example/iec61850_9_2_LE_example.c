/*
 *  iec61850_9_2_LE_example.c
 *
 *  Copyright 2016 Michael Zillgith
 *
 *  This file is part of libIEC61850.
 *
 *  libIEC61850 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libIEC61850 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See COPYING file for the complete license text.
 */

#include "iec61850_server.h"
#include "sv_publisher.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


/* Include the generated header with the model access handles */
#include "static_model.h"

/* import IEC 61850 device model created from SCL-File */
extern IedModel iedModel;

static int running = 0;
static int svcbEnabled = 1;

void sigint_handler(int signalId)
{
    running = 0;
}

static int amp1;
static int amp2;
static int amp3;
static int amp4;

static int vol1;
static int vol2;
static int vol3;
static int vol4;

static SVPublisher svPublisher;
static SVPublisher_ASDU asdu;

static void
setupSVPublisher(const char* svInterface)
{
    svPublisher = SVPublisher_create(NULL, svInterface);

    asdu = SVPublisher_addASDU(svPublisher, "xxxxMUnn01", NULL, 1);

    amp1 = SVPublisher_ASDU_addINT32(asdu);
    amp2 = SVPublisher_ASDU_addINT32(asdu);
    amp3 = SVPublisher_ASDU_addINT32(asdu);
    amp4 = SVPublisher_ASDU_addINT32(asdu);

    vol1 = SVPublisher_ASDU_addINT32(asdu);
    vol2 = SVPublisher_ASDU_addINT32(asdu);
    vol3 = SVPublisher_ASDU_addINT32(asdu);
    vol4 = SVPublisher_ASDU_addINT32(asdu);

    SVPublisher_setupComplete(svPublisher);
}

static void sVCBEventHandler (SVControlBlock* svcb, int event, void* parameter)
{
    if (event == IEC61850_SVCB_EVENT_ENABLE)
        svcbEnabled = 1;
    else if (event == IEC61850_SVCB_EVENT_DISABLE)
        svcbEnabled = 0;
}

int 
main(int argc, char** argv)
{
    char* svInterface;

    if (argc > 1)
        svInterface = argv[1];
    else
        svInterface = "eth0";

    IedServer iedServer = IedServer_create(&iedModel);

    /* MMS server will be instructed to start listening to client connections. */
    IedServer_start(iedServer, 102);

    if (!IedServer_isRunning(iedServer)) {
        printf("Starting server failed! Exit.\n");
        IedServer_destroy(iedServer);
        exit(-1);
    }

    running = 1;

    signal(SIGINT, sigint_handler);

    setupSVPublisher(svInterface);

    int voltage = 1;
    int current = 1;

    SVControlBlock* svcb = IedModel_getSVControlBlock(&iedModel, IEDMODEL_MUnn_LLN0, "MSVCB01");

    if (svcb == NULL) {
        printf("Lookup svcb failed!\n");
        exit(1);
    }

    IedServer_setSVCBHandler(iedServer, svcb, sVCBEventHandler, NULL);

    while (running) {

        uint64_t timeval = Hal_getTimeInMs();

        IedServer_lockDataModel(iedServer);

        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TCTR1_Amp_instMag_i, current);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TCTR2_Amp_instMag_i, current);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TCTR3_Amp_instMag_i, current);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TCTR3_Amp_instMag_i, current);

        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TVTR1_Vol_instMag_i, voltage);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TVTR2_Vol_instMag_i, voltage);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TVTR3_Vol_instMag_i, voltage);
        IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_MUnn_TVTR4_Vol_instMag_i, voltage);

        IedServer_unlockDataModel(iedServer);

        if (svcbEnabled) {

            SVPublisher_ASDU_setINT32(asdu, amp1, current);
            SVPublisher_ASDU_setINT32(asdu, amp2, current);
            SVPublisher_ASDU_setINT32(asdu, amp3, current);
            SVPublisher_ASDU_setINT32(asdu, amp4, current);

            SVPublisher_ASDU_setINT32(asdu, vol1, voltage);
            SVPublisher_ASDU_setINT32(asdu, vol2, voltage);
            SVPublisher_ASDU_setINT32(asdu, vol3, voltage);
            SVPublisher_ASDU_setINT32(asdu, vol4, voltage);

            SVPublisher_ASDU_increaseSmpCnt(asdu);

            SVPublisher_publish(svPublisher);
        }

        voltage++;
        current++;

        Thread_sleep(500);
    }

    /* stop MMS server - close TCP server socket and all client sockets */
    IedServer_stop(iedServer);

    /* Cleanup - free all resources */
    SVPublisher_destroy(svPublisher);
    IedServer_destroy(iedServer);

    return 0;
} /* main() */
