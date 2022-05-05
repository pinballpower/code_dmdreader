#include "drmhelper.h"

#include <boost/log/trivial.hpp>

int drmDeviceFd;
drmModeModeInfo drmMode;
drmModeCrtc* drmCrtc;
uint32_t drmConnectorId;

drmModeConnector* getDRMConnector(int drmDeviceFd, drmModeRes* resources, int displayNumber)
{
    int currentDisplay = 0;

    for (int i = 0; i < resources->count_connectors; i++)
    {
        drmModeConnector* connector = drmModeGetConnector(drmDeviceFd, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED)
        {
            if (currentDisplay == displayNumber) {
                return connector;
            }
            else {
                currentDisplay++;
            }
        }
        drmModeFreeConnector(connector);
    }

    return NULL;
}

drmModeEncoder* findDRMEncoder(int drmDeviceFd, drmModeConnector* connector)
{
    if (connector->encoder_id)
    {
        return drmModeGetEncoder(drmDeviceFd, connector->encoder_id);
    }
    return NULL;
}