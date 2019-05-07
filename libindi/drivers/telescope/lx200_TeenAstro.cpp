/*
    LX200 LX200_TeenAstro
    Based on LX200 TeenAstro, (alain@zwingelstein.org)
    Contributors:
    James Lan https://github.com/james-lan
    Ray Wells https://github.com/blueshawk
    Sébastien Durand https://github.com/dragonlost
    Copyright (C) 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "lx200_TeenAstro.h"

#define LIBRARY_TAB  "Library"
#define FIRMWARE_TAB "Firmware data"
#define STATUS_TAB "TeenAstro Status"
#define ALIGN_TAB "Align"
#define OUTPUT_TAB "Outputs"

#define TeenAstro_TIMEOUT  3

LX200_TeenAstro::LX200_TeenAstro() : LX200Generic(), FI(this)
{
    currentCatalog    = LX200_STAR_C;
    currentSubCatalog = 0;

    setVersion(1, 6);	// don't forget to update libindi/drivers.xml
    
    setLX200Capability(LX200_HAS_TRACKING_FREQ | LX200_HAS_SITES | LX200_HAS_ALIGNMENT_TYPE | LX200_HAS_PULSE_GUIDING | LX200_HAS_PRECISE_TRACKING_FREQ);
    
    SetTelescopeCapability(GetTelescopeCapability() | TELESCOPE_CAN_CONTROL_TRACK | TELESCOPE_HAS_PIER_SIDE | TELESCOPE_HAS_TRACK_RATE, 4 );
    
    //CAN_ABORT, CAN_GOTO ,CAN_PARK ,CAN_SYNC ,HAS_LOCATION ,HAS_TIME ,HAS_TRACK_MODE Already inherited from lx200generic,
    // 4 stands for the number of Slewrate Buttons as defined in Inditelescope.cpp
    //setLX200Capability(LX200_HAS_FOCUS | LX200_HAS_TRACKING_FREQ | LX200_HAS_ALIGNMENT_TYPE | LX200_HAS_SITES | LX200_HAS_PULSE_GUIDING);
    //
    // Get generic capabilities but discard the followng:
    // LX200_HAS_FOCUS

}

const char *LX200_TeenAstro::getDefaultName()
{
    return "LX200 TeenAstro";
}

bool LX200_TeenAstro::initProperties()
{

    LX200Generic::initProperties();
    FI::initProperties(FOCUS_TAB);
    SetParkDataType(PARK_RA_DEC);
    
    
    // ============== MAIN_CONTROL_TAB
    IUFillSwitch(&ReticS[0], "PLUS", "Light", ISS_OFF);
    IUFillSwitch(&ReticS[1], "MOINS", "Dark", ISS_OFF);
    IUFillSwitchVector(&ReticSP, ReticS, 2, getDeviceName(), "RETICULE_BRIGHTNESS", "Reticule +/-", MAIN_CONTROL_TAB, IP_RW, ISR_ATMOST1, 60, IPS_IDLE);

    IUFillNumber(&ElevationLimitN[0], "minAlt", "Elev Min", "%+03f", -90.0, 90.0, 1.0, -30.0);
    IUFillNumber(&ElevationLimitN[1], "maxAlt", "Elev Max", "%+03f", -90.0, 90.0, 1.0, 89.0);
    IUFillNumberVector(&ElevationLimitNP, ElevationLimitN, 2, getDeviceName(), "Slew elevation Limit", "", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    IUFillText(&ObjectInfoT[0], "Info", "", "");
    IUFillTextVector(&ObjectInfoTP, ObjectInfoT, 1, getDeviceName(), "Object Info", "", MAIN_CONTROL_TAB, IP_RO, 0, IPS_IDLE);

    // ============== CONNECTION_TAB

    // ============== OPTION_TAB

    // ============== MOTION_CONTROL_TAB

    IUFillNumber(&MaxSlewRateN[0], "maxSlew", "Rate", "%g", 1.0, 9.0, 1.0, 5.0);    //2.0, 9.0, 1.0, 9.0
    IUFillNumberVector(&MaxSlewRateNP, MaxSlewRateN, 1, getDeviceName(), "Max slew Rate", "", MOTION_TAB, IP_RW, 0,IPS_IDLE);

    IUFillSwitch(&TrackCompS[0], "1", "Full Compensation", ISS_OFF);
    IUFillSwitch(&TrackCompS[1], "2", "Refraction", ISS_OFF);
    IUFillSwitch(&TrackCompS[2], "3", "Off", ISS_OFF);
    IUFillSwitchVector(&TrackCompSP, TrackCompS, 3, getDeviceName(), "Compensation", "Compensation Tracking", MOTION_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);

    IUFillNumber(&BacklashN[0], "Backlash DEC", "DE", "%g", 0, 999, 1, 15);
    IUFillNumber(&BacklashN[1], "Backlash RA", "RA", "%g", 0, 999, 1, 15);
    IUFillNumberVector(&BacklashNP, BacklashN, 2, getDeviceName(), "Backlash", "", MOTION_TAB, IP_RW, 0,IPS_IDLE);
    
    IUFillSwitch(&AutoFlipS[0], "1", "AutoFlip: OFF", ISS_OFF);
    IUFillSwitch(&AutoFlipS[1], "2", "AutoFlip: ON", ISS_OFF);
    IUFillSwitchVector(&AutoFlipSP, AutoFlipS, 2, getDeviceName(), "AutoFlip", "Meridian Auto Flip", MOTION_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);
    
    IUFillSwitch(&HomePauseS[0], "1", "HomePause: OFF", ISS_OFF);
    IUFillSwitch(&HomePauseS[1], "2", "HomePause: ON", ISS_OFF);
    IUFillSwitch(&HomePauseS[2], "3", "HomePause: Continue", ISS_OFF);
    IUFillSwitchVector(&HomePauseSP, HomePauseS, 3, getDeviceName(), "HomePause", "Pause at Home", MOTION_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);
    
    IUFillSwitch(&FrequencyAdjustS[0], "1", "Frequency -", ISS_OFF);
    IUFillSwitch(&FrequencyAdjustS[1], "2", "Frequency +", ISS_OFF);
    IUFillSwitch(&FrequencyAdjustS[2], "3", "Reset Sidereal Frequency", ISS_OFF);
    IUFillSwitchVector(&FrequencyAdjustSP, FrequencyAdjustS, 3, getDeviceName(), "FrequencyAdjust", "Frequency Adjust", MOTION_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);
    
    IUFillSwitch(&PreferredPierSideS[0], "1", "West", ISS_OFF);
    IUFillSwitch(&PreferredPierSideS[1], "2", "East", ISS_OFF);
    IUFillSwitch(&PreferredPierSideS[2], "3", "Best", ISS_OFF);
    IUFillSwitchVector(&PreferredPierSideSP, PreferredPierSideS, 3, getDeviceName(), "Preferred Pier Side", "Preferred Pier Side", MOTION_TAB, IP_RW, ISR_1OFMANY, 0, IPS_IDLE);
    
    IUFillNumber(&minutesPastMeridianN[0], "East", "East", "%g", 0, 180, 1, 15);
    IUFillNumber(&minutesPastMeridianN[1], "West", "West", "%g", 0, 180, 1, 15);
    IUFillNumberVector(&minutesPastMeridianNP, minutesPastMeridianN, 2, getDeviceName(), "Minutes Past Meridian", "Minutes Past Meridian", MOTION_TAB, IP_RW, 0,IPS_IDLE);
    

    // ============== SITE_MANAGEMENT_TAB
    IUFillSwitch(&SetHomeS[0], "RETURN_HOME", "Return  Home", ISS_OFF);
    IUFillSwitch(&SetHomeS[1], "AT_HOME", "At Home (Reset)", ISS_OFF);
    IUFillSwitchVector(&SetHomeSP, SetHomeS, 2, getDeviceName(), "HOME_INIT", "Homing", SITE_TAB, IP_RW, ISR_ATMOST1, 60, IPS_IDLE);

    // ============== FIRMWARE_TAB
    IUFillText(&VersionT[0], "Date", "", "");
    IUFillText(&VersionT[1], "Time", "", "");
    IUFillText(&VersionT[2], "Number", "", "");
    IUFillText(&VersionT[3], "Name", "", "");
    IUFillTextVector(&VersionTP, VersionT, 4, getDeviceName(), "Firmware Info", "", FIRMWARE_TAB, IP_RO, 0, IPS_IDLE);

    // ============== New ALIGN_TAB 
    // Only supports Alpha versions currently (July 2018) Now Beta (Dec 2018)
    IUFillSwitch(&OSNAlignStarsS[0], "1", "1 Star", ISS_OFF);
    IUFillSwitch(&OSNAlignStarsS[1], "2", "2 Stars", ISS_OFF);
    IUFillSwitch(&OSNAlignStarsS[2], "3", "3 Stars", ISS_ON);
    IUFillSwitch(&OSNAlignStarsS[3], "4", "4 Stars", ISS_OFF);
    IUFillSwitch(&OSNAlignStarsS[4], "5", "5 Stars", ISS_OFF);
    IUFillSwitch(&OSNAlignStarsS[5], "6", "6 Stars", ISS_OFF);
    IUFillSwitch(&OSNAlignStarsS[6], "9", "9 Stars", ISS_OFF);
    IUFillSwitchVector(&OSNAlignStarsSP, OSNAlignStarsS, 7, getDeviceName(), "AlignStars", "Align using some stars, Alpha only", ALIGN_TAB, IP_RW, ISR_ATMOST1, 0, IPS_IDLE);
    
    IUFillSwitch(&OSNAlignS[0], "0", "Start Align", ISS_OFF);
    IUFillSwitch(&OSNAlignS[1], "1", "Issue Align", ISS_OFF);
    IUFillSwitch(&OSNAlignS[2], "3", "Write Align", ISS_OFF);
    IUFillSwitchVector(&OSNAlignSP, OSNAlignS, 3, getDeviceName(), "NewAlignStar", "Align using up to 6 stars, Alpha only", ALIGN_TAB, IP_RW, ISR_ATMOST1, 0, IPS_IDLE);
    
    IUFillText(&OSNAlignT[0], "0", "Align Process Status", "Align not started");
    IUFillText(&OSNAlignT[1], "1", "1. Manual Process", "Point towards the NCP");
    IUFillText(&OSNAlignT[2], "2", "2. Plate Solver Process", "Point towards the NCP");
    IUFillText(&OSNAlignT[3], "3", "Manual Action after 1", "Press 'Start Align'");
    IUFillText(&OSNAlignT[4], "4", "Current Status", "Not Updated");
    IUFillText(&OSNAlignT[5], "5", "Max Stars", "Not Updated");
    IUFillText(&OSNAlignT[6], "6", "Current Star", "Not Updated");
    IUFillText(&OSNAlignT[7], "7", "# of Align Stars", "Not Updated");
    IUFillTextVector(&OSNAlignTP, OSNAlignT, 8, getDeviceName(), "NAlign Process", "", ALIGN_TAB, IP_RO, 0, IPS_IDLE);
    
    IUFillText(&OSNAlignErrT[0], "0", "EQ Polar Error Alt", "Available once Aligned");
    IUFillText(&OSNAlignErrT[1], "1", "EQ Polar Error Az", "Available once Aligned");
//     IUFillText(&OSNAlignErrT[2], "2", "2. Plate Solver Process", "Point towards the NCP");
//     IUFillText(&OSNAlignErrT[3], "3", "After 1 or 2", "Press 'Start Align'");
//     IUFillText(&OSNAlignErrT[4], "4", "Current Status", "Not Updated");
//     IUFillText(&OSNAlignErrT[5], "5", "Max Stars", "Not Updated");
//     IUFillText(&OSNAlignErrT[6], "6", "Current Star", "Not Updated");
//     IUFillText(&OSNAlignErrT[7], "7", "# of Align Stars", "Not Updated");
    IUFillTextVector(&OSNAlignErrTP, OSNAlignErrT, 2, getDeviceName(), "ErrAlign Process", "", ALIGN_TAB, IP_RO, 0, IPS_IDLE);    
    
#ifdef TeenAstro_NOTDONE
    // =============== OUTPUT_TAB
    // =============== 
    IUFillSwitch(&OSOutput1S[0], "0", "OFF", ISS_ON);
    IUFillSwitch(&OSOutput1S[1], "1", "ON", ISS_OFF);
    IUFillSwitchVector(&OSOutput1SP, OSOutput1S, 2, getDeviceName(), "Output 1", "Output 1", OUTPUT_TAB, IP_RW, ISR_ATMOST1, 60, IPS_ALERT);
    
    IUFillSwitch(&OSOutput2S[0], "0", "OFF", ISS_ON);
    IUFillSwitch(&OSOutput2S[1], "1", "ON", ISS_OFF);
    IUFillSwitchVector(&OSOutput2SP, OSOutput2S, 2, getDeviceName(), "Output 2", "Output 2", OUTPUT_TAB, IP_RW, ISR_ATMOST1, 60, IPS_ALERT);
#endif
    
    // ============== STATUS_TAB
    IUFillText(&TeenAstroStat[0], ":GU# return", "", "");
    IUFillText(&TeenAstroStat[1], "Tracking", "", "");
    IUFillText(&TeenAstroStat[2], "Refractoring", "", "");
    IUFillText(&TeenAstroStat[3], "Park", "", "");
    IUFillText(&TeenAstroStat[5], "TimeSync", "", "");
    IUFillText(&TeenAstroStat[6], "Mount Type", "", "");
    IUFillText(&TeenAstroStat[7], "Error", "", "");
    IUFillTextVector(&TeenAstroStatTP, TeenAstroStat, 8, getDeviceName(), "TeenAstro Status", "", STATUS_TAB, IP_RO, 0, IPS_OK);

    return true;
}

void LX200_TeenAstro::ISGetProperties(const char *dev)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) != 0) return;
    LX200Generic::ISGetProperties(dev);
}

bool LX200_TeenAstro::updateProperties()
{
    LX200Generic::updateProperties();
    FI::updateProperties();

    if (isConnected())
    {
        // Firstinitialize some variables
        // keep sorted by TABs is easier
        // Main Control
        defineSwitch(&ReticSP);
        defineNumber(&ElevationLimitNP);
        defineText(&ObjectInfoTP);
        // Connection

        // Options

        // Motion Control
        defineNumber(&MaxSlewRateNP);
        defineSwitch(&TrackCompSP);
        defineNumber(&BacklashNP);
        defineSwitch(&AutoFlipSP);
        defineSwitch(&HomePauseSP);
        defineSwitch(&FrequencyAdjustSP);
	defineSwitch(&PreferredPierSideSP);
	defineNumber(&minutesPastMeridianNP);

        // Site Management
        defineSwitch(&ParkOptionSP);
        defineSwitch(&SetHomeSP);

        // Firmware Data
        defineText(&VersionTP);
	
        //New Align
        defineSwitch(&OSNAlignStarsSP);
        defineSwitch(&OSNAlignSP);
        defineText(&OSNAlignTP);
        defineText(&OSNAlignErrTP);
    #ifdef TeenAstro_NOTDONE
        //Outputs
        defineSwitch(&OSOutput1SP);
        defineSwitch(&OSOutput2SP);
    #endif
        // TeenAstro Status
        defineText(&TeenAstroStatTP);

        if (InitPark())
        {
            // If loading parking data is successful, we just set the default parking values.
            SetAxis1ParkDefault(LocationN[LOCATION_LATITUDE].value >= 0 ? 0 : 180);
            SetAxis2ParkDefault(LocationN[LOCATION_LATITUDE].value);
        }
        else
        {
            // Otherwise, we set all parking data to default in case no parking data is found.
            SetAxis1Park(LocationN[LOCATION_LATITUDE].value >= 0 ? 0 : 180);
            SetAxis1ParkDefault(LocationN[LOCATION_LATITUDE].value);

            SetAxis1ParkDefault(LocationN[LOCATION_LATITUDE].value >= 0 ? 0 : 180);
            SetAxis2ParkDefault(LocationN[LOCATION_LATITUDE].value);
         }

         double longitude=-1000, latitude=-1000;
         // Get value from config file if it exists.
         IUGetConfigNumber(getDeviceName(), "GEOGRAPHIC_COORD", "LONG", &longitude);
         IUGetConfigNumber(getDeviceName(), "GEOGRAPHIC_COORD", "LAT", &latitude);
         if (longitude != -1000 && latitude != -1000)
         {
             updateLocation(latitude, longitude, 0);
         }
    }
    else
    {
        // keep sorted by TABs is easier
        // Main Control
        deleteProperty(ReticSP.name);
        deleteProperty(ElevationLimitNP.name);
        // Connection

        // Options

        // Motion Control
        deleteProperty(MaxSlewRateNP.name);
        deleteProperty(TrackCompSP.name);
        deleteProperty(BacklashNP.name);
        deleteProperty(AutoFlipSP.name);
        deleteProperty(HomePauseSP.name);
        deleteProperty(FrequencyAdjustSP.name);
	deleteProperty(PreferredPierSideSP.name);
	deleteProperty(minutesPastMeridianNP.name);

        // Site Management
        deleteProperty(ParkOptionSP.name);
        deleteProperty(SetHomeSP.name);


        // Firmware Data
        deleteProperty(VersionTP.name);
	
        //New Align
        deleteProperty(OSNAlignStarsSP.name);
        deleteProperty(OSNAlignSP.name);
        deleteProperty(OSNAlignTP.name);
        deleteProperty(OSNAlignErrTP.name);
    #ifdef TeenAstro_NOTDONE
        //Outputs
        deleteProperty(OSOutput1SP.name);
        deleteProperty(OSOutput2SP.name);
    #endif

        // TeenAstro Status
        deleteProperty(TeenAstroStatTP.name);
    }
    return true;
}

bool LX200_TeenAstro::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
	if (strstr(name, "FOCUS_"))
		    return FI::processNumber(dev, name, values, names, n);
        if (!strcmp(name, ObjectNoNP.name))
        {
            char object_name[256];

            if (selectCatalogObject(PortFD, currentCatalog, (int)values[0]) < 0)
            {
                ObjectNoNP.s = IPS_ALERT;
                IDSetNumber(&ObjectNoNP, "Failed to select catalog object.");
                return false;
            }

            getLX200RA(PortFD, &targetRA);
            getLX200DEC(PortFD, &targetDEC);

            ObjectNoNP.s = IPS_OK;
            IDSetNumber(&ObjectNoNP, "Object updated.");

            if (getObjectInfo(PortFD, object_name) < 0)
                IDMessage(getDeviceName(), "Getting object info failed.");
            else
            {
                IUSaveText(&ObjectInfoTP.tp[0], object_name);
                IDSetText(&ObjectInfoTP, nullptr);
            }
            Goto(targetRA, targetDEC);
            return true;
        }

        if (!strcmp(name, MaxSlewRateNP.name))
        {
            int ret;
            char cmd[4];
            snprintf(cmd, 4, ":R%d#", (int)values[0]);
            ret = sendTeenAstroCommandBlind(cmd);

            //if (setMaxSlewRate(PortFD, (int)values[0]) < 0) //(int) MaxSlewRateN[0].value
            if (ret == -1)
            {
                LOGF_DEBUG("Pas OK Return value =%d", ret);
                LOGF_DEBUG("Setting Max Slew Rate to %f\n", values[0]);
                MaxSlewRateNP.s = IPS_ALERT;
                IDSetNumber(&MaxSlewRateNP, "Setting Max Slew Rate Failed");
                return false;
            }
            LOGF_DEBUG("OK Return value =%d", ret);
            MaxSlewRateNP.s           = IPS_OK;
            MaxSlewRateNP.np[0].value = values[0];
            IDSetNumber(&MaxSlewRateNP, "Slewrate set to %04.1f", values[0]);
            return true;
        }

        if (!strcmp(name, BacklashNP.name))
        {
            char cmd[9];
            int i, nset;
            double bklshdec=0, bklshra=0;

            for (nset = i = 0; i < n; i++)
            {
                INumber *bktp = IUFindNumber(&BacklashNP, names[i]);
                if (bktp == &BacklashN[0])
                {
                    bklshdec = values[i];
                    LOGF_DEBUG("===CMD==> Backlash DEC= %f", bklshdec);
                    nset += bklshdec >= 0 && bklshdec <= 999;  //range 0 to 999
                }
                else if (bktp == &BacklashN[1])
                {
                    bklshra = values[i];
                    LOGF_DEBUG("===CMD==> Backlash RA= %f", bklshra);
                    nset += bklshra >= 0 && bklshra <= 999;   //range 0 to 999
                }
            }
            if (nset == 2)
            {
                snprintf(cmd, 9, ":$BD%d#", (int)bklshdec);
                if (sendTeenAstroCommand(cmd))
                {
                    BacklashNP.s = IPS_ALERT;
                    IDSetNumber(&BacklashNP, "Error Backlash DEC limit.");
                }
                const struct timespec timeout = {0, 100000000L};
                nanosleep(&timeout, nullptr); // time for TeenAstro to respond to previous cmd
                snprintf(cmd, 9, ":$BR%d#", (int)bklshra);
                if (sendTeenAstroCommand(cmd))
                {
                    BacklashNP.s = IPS_ALERT;
                    IDSetNumber(&BacklashNP, "Error Backlash RA limit.");
                }

                BacklashNP.np[0].value = bklshdec;
                BacklashNP.np[1].value = bklshra;
                BacklashNP.s           = IPS_OK;
                IDSetNumber(&BacklashNP, nullptr);
                return true;
            }
            else
            {
                BacklashNP.s = IPS_ALERT;
                IDSetNumber(&BacklashNP, "Backlash invalid.");
                return false;
            }
        }

        if (!strcmp(name, ElevationLimitNP.name))
        {
            // new elevation limits
            double minAlt = 0, maxAlt = 0;
            int i, nset;

            for (nset = i = 0; i < n; i++)
            {
                INumber *altp = IUFindNumber(&ElevationLimitNP, names[i]);
                if (altp == &ElevationLimitN[0])
                {
                    minAlt = values[i];
                    nset += minAlt >= -30.0 && minAlt <= 30.0;  //range -30 to 30
                }
                else if (altp == &ElevationLimitN[1])
                {
                    maxAlt = values[i];
                    nset += maxAlt >= 60.0 && maxAlt <= 90.0;   //range 60 to 90
                }
            }
            if (nset == 2)
            {
                if (setMinElevationLimit(PortFD, (int)minAlt) < 0)
                {
                    ElevationLimitNP.s = IPS_ALERT;
                    IDSetNumber(&ElevationLimitNP, "Error setting min elevation limit.");
                }

                if (setMaxElevationLimit(PortFD, (int)maxAlt) < 0)
                {
                    ElevationLimitNP.s = IPS_ALERT;
                    IDSetNumber(&ElevationLimitNP, "Error setting max elevation limit.");
                    return false;
                }
                ElevationLimitNP.np[0].value = minAlt;
                ElevationLimitNP.np[1].value = maxAlt;
                ElevationLimitNP.s           = IPS_OK;
                IDSetNumber(&ElevationLimitNP, nullptr);
                return true;
            }
            else
            {
                ElevationLimitNP.s = IPS_IDLE;
                IDSetNumber(&ElevationLimitNP, "elevation limit missing or invalid.");
                return false;
            }
        }
    }

    if (!strcmp(name, minutesPastMeridianNP.name))  
    {
	    char cmd[20];
	    int i, nset;
	    double minPMEast=0, minPMWest=0;
	    
	    for (nset = i = 0; i < n; i++)
	    {
		    INumber *bktp = IUFindNumber(&minutesPastMeridianNP, names[i]);
		    if (bktp == &minutesPastMeridianN[0])
		    {
			    minPMEast = values[i];
			    LOGF_DEBUG("===CMD==> minutesPastMeridianN[0]/East = %f", minPMEast);
			    nset += minPMEast >= 0 && minPMEast <= 180;  //range 0 to 180
		    }
		    else if (bktp == &minutesPastMeridianN[1])
		    {
			    minPMWest = values[i];
			    LOGF_DEBUG("===CMD==> minutesPastMeridianN[1]/West= %f", minPMWest);
			    nset += minPMWest >= 0 && minPMWest <= 180;   //range 0 to 180
		    }
	    }
	    if (nset == 2)
	    {
		    snprintf(cmd, 20, ":SXE9,%d#", (int) minPMEast);
		    if (sendTeenAstroCommand(cmd))
		    {
			    minutesPastMeridianNP.s = IPS_ALERT;
			    IDSetNumber(&minutesPastMeridianNP, "Error Backlash DEC limit.");
		    }
		    const struct timespec timeout = {0, 100000000L};
		    nanosleep(&timeout, nullptr); // time for TeenAstro to respond to previous cmd
		    snprintf(cmd, 20, ":SXEA,%d#", (int) minPMWest);
		    if (sendTeenAstroCommand(cmd))
		    {
			    minutesPastMeridianNP.s = IPS_ALERT;
			    IDSetNumber(&minutesPastMeridianNP, "Error Backlash RA limit.");
		    }
		    
		    minutesPastMeridianNP.np[0].value = minPMEast;
		    minutesPastMeridianNP.np[1].value = minPMWest;
		    minutesPastMeridianNP.s           = IPS_OK;
		    IDSetNumber(&minutesPastMeridianNP, nullptr);
		    return true;
	    }
	    else
	    {
		    minutesPastMeridianNP.s = IPS_ALERT;
		    IDSetNumber(&minutesPastMeridianNP, "minutesPastMeridian invalid.");
		    return false;
	    }
    }

    return LX200Generic::ISNewNumber(dev, name, values, names, n);
}

bool LX200_TeenAstro::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    int index = 0;

    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Reticlue +/- Buttons
        if (!strcmp(name, ReticSP.name))
        {
            long ret = 0;

            IUUpdateSwitch(&ReticSP, states, names, n);
            ReticSP.s = IPS_OK;

            if (ReticS[0].s == ISS_ON)
            {
                ret = ReticPlus(PortFD);
                ReticS[0].s=ISS_OFF;
                IDSetSwitch(&ReticSP, "Bright");
            }
            else
            {
                ret = ReticMoins(PortFD);
                ReticS[1].s=ISS_OFF;
                IDSetSwitch(&ReticSP, "Dark");
            }

            IUResetSwitch(&ReticSP);
            IDSetSwitch(&ReticSP, nullptr);
            return true;
        }

        // Homing, Cold and Warm Init
        if (!strcmp(name, SetHomeSP.name))
        {
            IUUpdateSwitch(&SetHomeSP, states, names, n);
            SetHomeSP.s = IPS_OK;

            if (SetHomeS[0].s == ISS_ON)
            {
                if(!sendTeenAstroCommandBlind(":hC#"))
                    return false;
                IDSetSwitch(&SetHomeSP, "Return Home");
                SetHomeS[0].s = ISS_OFF;
            }
            else
            {
                if(!sendTeenAstroCommandBlind(":hF#"))
                    return false;
                IDSetSwitch(&SetHomeSP, "At Home (Reset)");
                SetHomeS[1].s = ISS_OFF;
            }
            IUResetSwitch(&ReticSP);
            SetHomeSP.s = IPS_IDLE;
            IDSetSwitch(&SetHomeSP, nullptr);
            return true;
        }

        // Tracking Compensation selection
        if (!strcmp(name, TrackCompSP.name))
        {
            IUUpdateSwitch(&TrackCompSP, states, names, n);
            TrackCompSP.s = IPS_BUSY;

            if (TrackCompS[0].s == ISS_ON)
            {
                if (!sendTeenAstroCommand(":To#"))
                {
                    IDSetSwitch(&TrackCompSP, "Full Compensated Tracking On");
		    TrackCompSP.s = IPS_OK;
		    IDSetSwitch(&TrackCompSP, nullptr);
                    return true;
                }
            }
            if (TrackCompS[1].s == ISS_ON)
            {
                if (!sendTeenAstroCommand(":Tr#"))
                {
                    IDSetSwitch(&TrackCompSP, "Refraction Tracking On");
		    TrackCompSP.s = IPS_OK;
		    IDSetSwitch(&TrackCompSP, nullptr);
                    return true;
                }
            }
            if (TrackCompS[2].s == ISS_ON)
            {
                if (!sendTeenAstroCommand(":Tn#"))
                {
                    IDSetSwitch(&TrackCompSP, "Refraction Tracking Disabled");
		    TrackCompSP.s = IPS_OK;
		    IDSetSwitch(&TrackCompSP, nullptr);
                    return true;
                }
            }
            IUResetSwitch(&TrackCompSP);
            TrackCompSP.s = IPS_IDLE;
            IDSetSwitch(&TrackCompSP, nullptr);
            return true;
        }
        if (!strcmp(name, AutoFlipSP.name))
	{
		IUUpdateSwitch(&AutoFlipSP, states, names, n);
		AutoFlipSP.s = IPS_BUSY;
		
		if (AutoFlipS[0].s == ISS_ON)
		{
			if (sendTeenAstroCommand(":SX95,0#"))
			{
				AutoFlipSP.s = IPS_OK;
				IDSetSwitch(&AutoFlipSP, "Auto Meridan Flip OFF");
				return true;
			} 
		}
		if (AutoFlipS[1].s == ISS_ON)
		{
			if (sendTeenAstroCommand(":SX95,1#"))
			{
				AutoFlipSP.s = IPS_OK;
				IDSetSwitch(&AutoFlipSP, "Auto Meridan Flip ON");
				return true;
			}
		}
		IUResetSwitch(&AutoFlipSP);
		//AutoFlipSP.s = IPS_IDLE;
		IDSetSwitch(&AutoFlipSP, nullptr);
		return true;
	}
        
        if (!strcmp(name, HomePauseSP.name))
	{
		IUUpdateSwitch(&HomePauseSP, states, names, n);
		HomePauseSP.s = IPS_BUSY;
		
		if (HomePauseS[0].s == ISS_ON)
		{
			if (sendTeenAstroCommand(":SX98,0#"))
			{
				HomePauseSP.s = IPS_OK;
				IDSetSwitch(&HomePauseSP, "Home Pause OFF");
				return true;
			} 
		}
		if (HomePauseS[1].s == ISS_ON)
		{
			if (sendTeenAstroCommand(":SX98,1#"))
			{
				HomePauseSP.s = IPS_OK;
				IDSetSwitch(&HomePauseSP, "Home Pause ON");
				return true;
			}
		}
		if (HomePauseS[2].s == ISS_ON)
		{
			if (sendTeenAstroCommand(":SX99,1#"))
			{
				IUResetSwitch(&HomePauseSP);
				HomePauseSP.s = IPS_OK;
				IDSetSwitch(&HomePauseSP, "Home Pause: Continue");
				return true;
			}
		}
		IUResetSwitch(&HomePauseSP);
		HomePauseSP.s = IPS_IDLE;
		IDSetSwitch(&HomePauseSP, nullptr);
		return true;
	}        
        
        if (!strcmp(name, FrequencyAdjustSP.name))      // 
		
	//
	{
		IUUpdateSwitch(&FrequencyAdjustSP, states, names, n);
		FrequencyAdjustSP.s = IPS_OK;
		
		if (FrequencyAdjustS[0].s == ISS_ON)
		{
			if (!sendTeenAstroCommandBlind(":T-#"))
			{
				IDSetSwitch(&FrequencyAdjustSP, "Frequency decreased");
				return true;
			}
		}
		if (FrequencyAdjustS[1].s == ISS_ON)
		{
			if (!sendTeenAstroCommandBlind(":T+#"))
			{
				IDSetSwitch(&FrequencyAdjustSP, "Frequency increased");
				return true;
			}
		}
		if (FrequencyAdjustS[2].s == ISS_ON)
		{
			if (!sendTeenAstroCommandBlind(":TR#"))
			{
				IDSetSwitch(&FrequencyAdjustSP, "Frequency Reset (TO saved EEPROM)");
				return true;
			}
		}
		IUResetSwitch(&FrequencyAdjustSP);
		FrequencyAdjustSP.s = IPS_IDLE;
		IDSetSwitch(&FrequencyAdjustSP, nullptr);
		return true;
	}
	
	//Pier Side
	if (!strcmp(name, PreferredPierSideSP.name))
	{
		IUUpdateSwitch(&PreferredPierSideSP, states, names, n);
		PreferredPierSideSP.s = IPS_BUSY;
		
		if (PreferredPierSideS[0].s == ISS_ON) //West
		{
			if (sendTeenAstroCommand(":SX96,W#"))
			{
				PreferredPierSideSP.s = IPS_OK;
				IDSetSwitch(&PreferredPierSideSP, "Preferred Pier Side: West");
				return true;
			} 
		}
		if (PreferredPierSideS[1].s == ISS_ON) //East
		{
			if (sendTeenAstroCommand(":SX96,E#"))
			{
				PreferredPierSideSP.s = IPS_OK;
				IDSetSwitch(&PreferredPierSideSP, "Preferred Pier Side: East");
				return true;
			}
		}
		if (PreferredPierSideS[2].s == ISS_ON) //Best
		{
			if (sendTeenAstroCommand(":SX96,B#"))
			{
				PreferredPierSideSP.s = IPS_OK;
				IDSetSwitch(&PreferredPierSideSP, "Preferred Pier Side: Best");
				return true;
			}
		}
		IUResetSwitch(&PreferredPierSideSP);
		IDSetSwitch(&PreferredPierSideSP, nullptr);
		return true;
	}
	}

	// Align Buttons
	if (!strcmp(name, OSNAlignStarsSP.name))
	{
		IUResetSwitch(&OSNAlignStarsSP);
		IUUpdateSwitch(&OSNAlignStarsSP, states, names, n);
		index = IUFindOnSwitchIndex(&OSNAlignStarsSP);
		
		return true;
	}

    // Alignment
    if (!strcmp(name, OSNAlignSP.name))
	{
		if (IUUpdateSwitch(&OSNAlignSP, states, names, n) < 0)
			return false;
		
		index = IUFindOnSwitchIndex(&OSNAlignSP);
		//NewGeometricAlignment    
		//End NewGeometricAlignment 
		OSNAlignSP.s = IPS_BUSY;
		if (index == 0)
		{    
			
			/* From above. Could be added to have 7,8 star
			IUFillSwitch(&OSNAlignStarsS[0], "1", "1 Star", ISS_OFF);
			IUFillSwitch(&OSNAlignStarsS[1], "2", "2 Stars", ISS_OFF);*
			IUFillSwitch(&OSNAlignStarsS[2], "3", "3 Stars", ISS_ON);
			IUFillSwitch(&OSNAlignStarsS[3], "4", "4 Stars", ISS_OFF);
			IUFillSwitch(&OSNAlignStarsS[4], "5", "5 Stars", ISS_OFF);
			IUFillSwitch(&OSNAlignStarsS[5], "6", "6 Stars", ISS_OFF);
			IUFillSwitch(&OSNAlignStarsS[6], "9", "9 Stars", ISS_OFF);*/
			
			int index_stars = IUFindOnSwitchIndex(&OSNAlignStarsSP);
			if ((index_stars <= 6) && (index_stars >= 0)) {
				int stars = index_stars+1;
				if (stars == 6) stars = 9;
				//if (stars == 5) stars = 6;
				OSNAlignS[0].s = ISS_OFF;
				LOGF_INFO("Align index: %d, stars: %d", index_stars, stars); 
				AlignStartGeometric(stars);
			}
		}
		if (index == 1)
		{
			OSNAlignS[1].s = ISS_OFF;
			OSNAlignSP.s = AlignAddStar();
		}
		if (index == 2)
		{
			OSNAlignS[2].s = ISS_OFF;
			OSNAlignSP.s = AlignDone();
		}
		IDSetSwitch(&OSNAlignSP, nullptr);
		UpdateAlignStatus();
	}

#ifdef TeenAstro_NOTDONE
	if (!strcmp(name, OSOutput1SP.name))      // 
	{
		if (OSOutput1S[0].s == ISS_ON)
		{
			OSDisableOutput(1);
			//PECStateS[0].s == ISS_OFF;
		} else if (OSOutput1S[1].s == ISS_ON)
		{
			OSEnableOutput(1);
			//PECStateS[1].s == ISS_OFF;
		}
		IDSetSwitch(&OSOutput1SP, nullptr);
	}
	if (!strcmp(name, OSOutput2SP.name))      // 
	{
		if (OSOutput2S[0].s == ISS_ON)
		{
			OSDisableOutput(2);
			//PECStateS[0].s == ISS_OFF;
		} else if (OSOutput2S[1].s == ISS_ON)
		{
			OSEnableOutput(2);
			//PECStateS[1].s == ISS_OFF;
		}
		IDSetSwitch(&OSOutput2SP, nullptr);
	}
#endif
	
    }

    return LX200Generic::ISNewSwitch(dev, name, states, names, n);
}

void LX200_TeenAstro::getBasicData()
{
    // process parent
    LX200Generic::getBasicData();

    if (!isSimulation())
    {
        char buffer[128];
        getVersionDate(PortFD, buffer);
        IUSaveText(&VersionT[0], buffer);
        getVersionTime(PortFD, buffer);
        IUSaveText(&VersionT[1], buffer);
        getVersionNumber(PortFD, buffer);
        IUSaveText(&VersionT[2], buffer);
        getProductName(PortFD, buffer);
        IUSaveText(&VersionT[3], buffer);

        IDSetText(&VersionTP, nullptr);

            if (InitPark())
            {
                // If loading parking data is successful, we just set the default parking values.
                LOG_INFO("=============== Parkdata loaded");
            }
            else
            {
                // Otherwise, we set all parking data to default in case no parking data is found.
                LOG_INFO("=============== Parkdata Load Failed");
            }
    }
}

//======================== Parking =======================
bool LX200_TeenAstro::SetCurrentPark()
{
    char response[RB_MAX_LEN];

    if(!getCommandString(PortFD, response, ":hQ#"))
        {
            LOGF_WARN("===CMD==> Set Park Pos %s", response);
            return false;
        }
    SetAxis1Park(currentRA);
    SetAxis2Park(currentDEC);
    LOG_WARN("Park Value set to current postion");
    return true;
}

bool LX200_TeenAstro::SetDefaultPark()
{
    IDMessage(getDeviceName(), "Setting Park Data to Default.");
    SetAxis1Park(20);
    SetAxis2Park(80);
    LOG_WARN("Park Position set to Default value, 20/80");
    return true;
}

bool LX200_TeenAstro::UnPark()
{
    char response[RB_MAX_LEN];


    if (!isSimulation())
    {
        if(!getCommandString(PortFD, response, ":hR#"))
        {
            return false;
        }
     }
    return true;
}

bool LX200_TeenAstro::Park()
{
    if (!isSimulation())
    {
        // If scope is moving, let's stop it first.
        if (EqNP.s == IPS_BUSY)
        {
            if (!isSimulation() && abortSlew(PortFD) < 0)
            {
                Telescope::AbortSP.s = IPS_ALERT;
                IDSetSwitch(&(Telescope::AbortSP), "Abort slew failed.");
                return false;
            }
            Telescope::AbortSP.s = IPS_OK;
            EqNP.s    = IPS_IDLE;
            IDSetSwitch(&(Telescope::AbortSP), "Slew aborted.");
            IDSetNumber(&EqNP, nullptr);

            if (MovementNSSP.s == IPS_BUSY || MovementWESP.s == IPS_BUSY)
            {
                MovementNSSP.s = MovementWESP.s = IPS_IDLE;
                EqNP.s                          = IPS_IDLE;
                IUResetSwitch(&MovementNSSP);
                IUResetSwitch(&MovementWESP);

                IDSetSwitch(&MovementNSSP, nullptr);
                IDSetSwitch(&MovementWESP, nullptr);
            }
        }
        if (!isSimulation() && slewToPark(PortFD) < 0)
        {
            ParkSP.s = IPS_ALERT;
            IDSetSwitch(&ParkSP, "Parking Failed.");
            return false;
        }
    }
    ParkSP.s   = IPS_BUSY;
    return true;
}

// Periodically Polls TeenAstro Parameter from controller
bool LX200_TeenAstro::ReadScopeStatus()
{
    char OSbacklashDEC[RB_MAX_LEN];
    char OSbacklashRA[RB_MAX_LEN];
    char TempValue[RB_MAX_LEN];
    char TempValue2[RB_MAX_LEN];
    Errors Lasterror = ERR_NONE;

    if (isSimulation()) //if Simulation is selected
    {
        mountSim();
        return true;
    }

    if (getLX200RA(PortFD, &currentRA) < 0 || getLX200DEC(PortFD, &currentDEC) < 0) // Update actual position
    {
        EqNP.s = IPS_ALERT;
        IDSetNumber(&EqNP, "Error reading RA/DEC.");
        return false;
    }

    getCommandString(PortFD,OSStat,":GU#"); // :GU# returns a string containg controller status
    if (strcmp(OSStat,OldOSStat) != 0)  //if status changed
    {
    // ============= Telescope Status
    strcpy(OldOSStat ,OSStat);

    IUSaveText(&TeenAstroStat[0],OSStat);
    if (strstr(OSStat,"n") && strstr(OSStat,"N"))
    {
        IUSaveText(&TeenAstroStat[1],"Idle");
        TrackState=SCOPE_IDLE;
    }
    if (strstr(OSStat,"n") && !strstr(OSStat,"N"))
    {
        IUSaveText(&TeenAstroStat[1],"Slewing");
        TrackState=SCOPE_SLEWING;
    }
    if (strstr(OSStat,"N") && !strstr(OSStat,"n"))
    {
        IUSaveText(&TeenAstroStat[1],"Tracking");
        TrackState=SCOPE_TRACKING;
    }

    // ============= Refractoring
    if (strstr(OSStat,"r")) {IUSaveText(&TeenAstroStat[2],"Refractoring On"); }
    if (strstr(OSStat,"s")) {IUSaveText(&TeenAstroStat[2],"Refractoring Off"); }
    if (strstr(OSStat,"r") && strstr(OSStat,"t")) {IUSaveText(&TeenAstroStat[2],"Full Comp"); }
    if (strstr(OSStat,"r") && !strstr(OSStat,"t")) { IUSaveText(&TeenAstroStat[2],"Refractory Comp"); }

    // ============= Parkstatus
    if(FirstRead)   // it is the first time I read the status so I need to update
    {
        if (strstr(OSStat,"P"))
        {
            SetParked(true); //defaults to TrackState=SCOPE_PARKED
            IUSaveText(&TeenAstroStat[3],"Parked");
         }
        if (strstr(OSStat,"F"))
        {
            SetParked(false); // defaults to TrackState=SCOPE_IDLE
            IUSaveText(&TeenAstroStat[3],"Parking Failed");
        }
        if (strstr(OSStat,"I"))
        {
            SetParked(false); //defaults to TrackState=SCOPE_IDLE but we want
            TrackState=SCOPE_PARKING;
            IUSaveText(&TeenAstroStat[3],"Park in Progress");
        }
        if (strstr(OSStat,"p"))
        {
            SetParked(false); //defaults to TrackState=SCOPE_IDLE but we want
            if (strstr(OSStat,"nN"))    // azwing need to detect if unparked idle or tracking
            {
                IUSaveText(&TeenAstroStat[1],"Idle");
                TrackState=SCOPE_IDLE;
            }
            else TrackState=SCOPE_TRACKING;
            IUSaveText(&TeenAstroStat[3],"UnParked");
        }
    FirstRead=false;
    }
    else
    {
        if (!isParked())
        {
            if(strstr(OSStat,"P"))
            {
                SetParked(true);
                IUSaveText(&TeenAstroStat[3],"Parked");
                //LOG_INFO("TeenAstro Parking Succeded");
            }
            if (strstr(OSStat,"I"))
            {
                SetParked(false); //defaults to TrackState=SCOPE_IDLE but we want
                TrackState=SCOPE_PARKING;
                IUSaveText(&TeenAstroStat[3],"Park in Progress");
                LOG_INFO("TeenAstro Parking in Progress...");
            }
        }
        if (isParked())
        {
            if (strstr(OSStat,"F"))
            {
                // keep Status even if error  TrackState=SCOPE_IDLE;
                SetParked(false); //defaults to TrackState=SCOPE_IDLE
                IUSaveText(&TeenAstroStat[3],"Parking Failed");
                LOG_ERROR("TeenAstro Parking failed, need to re Init TeenAstro at home");
            }
            if (strstr(OSStat,"p"))
            {
                SetParked(false); //defaults to TrackState=SCOPE_IDLE but we want
                if (strstr(OSStat,"nN"))    // azwing need to detect if unparked idle or tracking
                {
                    IUSaveText(&TeenAstroStat[1],"Idle");
                    TrackState=SCOPE_IDLE;
                }
                else TrackState=SCOPE_TRACKING;
                IUSaveText(&TeenAstroStat[3],"UnParked");
                //LOG_INFO("TeenAstro Unparked...");
            }
        }
    }

      
    //if (strstr(OSStat,"H")) { IUSaveText(&TeenAstroStat[3],"At Home"); }
    if (strstr(OSStat,"H") && strstr(OSStat,"P"))
    {
        IUSaveText(&TeenAstroStat[3],"At Home and Parked");
    }
    if (strstr(OSStat,"H") && strstr(OSStat,"p"))
    {
        IUSaveText(&TeenAstroStat[3],"At Home and UnParked");
    }
    //AutoPauseAtHome
    if (strstr(OSStat, "u")){ //  pa[u]se at home enabled?
	    HomePauseS[1].s = ISS_ON;
	    HomePauseSP.s = IPS_OK;
	    IDSetSwitch(&HomePauseSP, "Pause at Home Enabled");
    } else {
	    HomePauseS[0].s=ISS_ON;
	    HomePauseSP.s = IPS_OK;
	    IDSetSwitch(&HomePauseSP, nullptr);
    }    
    
    if (strstr(OSStat,"w")) { IUSaveText(&TeenAstroStat[3],"Waiting at Home"); }

    // ============= Time Sync Status
    if (!strstr(OSStat,"S")) { IUSaveText(&TeenAstroStat[5],"N/A"); }
    if (strstr(OSStat,"S")) { IUSaveText(&TeenAstroStat[5],"PPS / GPS Sync Ok"); }

    // ============= Mount Types
    if (strstr(OSStat,"E")) { IUSaveText(&TeenAstroStat[6],"German Mount"); }
    if (strstr(OSStat,"K")) { IUSaveText(&TeenAstroStat[6],"Fork Mount"); }
    if (strstr(OSStat,"k")) { IUSaveText(&TeenAstroStat[6],"Fork Alt Mount"); }
    if (strstr(OSStat,"A")) { IUSaveText(&TeenAstroStat[6],"AltAZ Mount"); }

    // ============= Error Code ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM, ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC, ERR_PARK, ERR_GOTO_SYNC
    Lasterror=(Errors)(OSStat[strlen(OSStat)-1]-'0');
    if (Lasterror==ERR_NONE) { IUSaveText(&TeenAstroStat[7],"None"); }
    if (Lasterror==ERR_MOTOR_FAULT) { IUSaveText(&TeenAstroStat[7],"Motor Fault"); }
    if (Lasterror==ERR_ALT) { IUSaveText(&TeenAstroStat[7],"Altitude Min/Max"); }
    if (Lasterror==ERR_LIMIT_SENSE) { IUSaveText(&TeenAstroStat[7],"Limit Sense"); }
    if (Lasterror==ERR_DEC) { IUSaveText(&TeenAstroStat[7],"Dec Limit Exceeded"); }
    if (Lasterror==ERR_AZM) { IUSaveText(&TeenAstroStat[7],"Azm Limit Exceeded"); }
    if (Lasterror==ERR_UNDER_POLE) { IUSaveText(&TeenAstroStat[7],"Under Pole Limit Exceeded"); }
    if (Lasterror==ERR_MERIDIAN) { IUSaveText(&TeenAstroStat[7],"Meridian Limit (W) Exceeded"); }
    if (Lasterror==ERR_SYNC) { IUSaveText(&TeenAstroStat[7],"Sync. ignored > 30 deg"); }
    if (Lasterror==ERR_PARK) { IUSaveText(&TeenAstroStat[7],"Park Error"); }
    if (Lasterror==ERR_GOTO_SYNC) { IUSaveText(&TeenAstroStat[7],"Goto Sync Error"); }
    }

    // Get actual Pier Side
    getCommandString(PortFD,OSPier,":Gm#");
    if (strcmp(OSPier, OldOSPier) !=0)  // any change ?
    {
        strcpy(OldOSPier, OSPier);
        switch(OSPier[0])
        {
        case 'E':
            setPierSide(PIER_EAST);
        break;

        case 'W':
            setPierSide(PIER_WEST);
        break;

        case 'N':
            setPierSide(PIER_UNKNOWN);
        break;

        case '?':
            setPierSide(PIER_UNKNOWN);
        break;
        }
    }

    //========== Get actual Backlash values
    getCommandString(PortFD,OSbacklashDEC, ":%BD#");
    getCommandString(PortFD,OSbacklashRA, ":%BR#");
    BacklashNP.np[0].value = atof(OSbacklashDEC);
    BacklashNP.np[1].value = atof(OSbacklashRA);
    IDSetNumber(&BacklashNP, nullptr);
    
    getCommandString(PortFD,OSbacklashDEC, ":%BD#");
    getCommandString(PortFD,OSbacklashRA, ":%BR#");
    BacklashNP.np[0].value = atof(OSbacklashDEC);
    BacklashNP.np[1].value = atof(OSbacklashRA);
    IDSetNumber(&BacklashNP, nullptr);
    
    //AutoFlip
    getCommandString(PortFD,TempValue,":GX95#");
    if (atoi(TempValue)) {
	AutoFlipS[1].s = ISS_ON;
	AutoFlipSP.s = IPS_OK;
	IDSetSwitch(&AutoFlipSP, nullptr);
    } else {
	AutoFlipS[0].s=ISS_ON;
	AutoFlipSP.s = IPS_OK;
	IDSetSwitch(&AutoFlipSP, nullptr);
    }
    
    //PreferredPierSide
    getCommandString(PortFD,TempValue,":GX96#");
    if (strstr(TempValue,"W")) {
	PreferredPierSideS[0].s = ISS_ON;
	PreferredPierSideSP.s = IPS_OK;
	IDSetSwitch(&PreferredPierSideSP, nullptr);
    } else if (strstr(TempValue,"E")) {
	PreferredPierSideS[1].s=ISS_ON;
	PreferredPierSideSP.s = IPS_OK;
	IDSetSwitch(&PreferredPierSideSP, nullptr);
    } else if (strstr(TempValue,"B")) {
	PreferredPierSideS[2].s=ISS_ON;
	PreferredPierSideSP.s = IPS_OK;
	IDSetSwitch(&PreferredPierSideSP, nullptr);
    } else {
	IUResetSwitch(&PreferredPierSideSP);
	PreferredPierSideSP.s = IPS_BUSY;
	IDSetSwitch(&PreferredPierSideSP, nullptr);
    }

    
    getCommandString(PortFD,TempValue, ":GXE9#"); // E
    getCommandString(PortFD,TempValue2, ":GXEA#"); // W 
    minutesPastMeridianNP.np[0].value = atof(TempValue); // E
    minutesPastMeridianNP.np[1].value = atof(TempValue2); //W
    IDSetNumber(&minutesPastMeridianNP, nullptr);

    // Update TeenAstro Status TAB
    IDSetText(&TeenAstroStatTP, nullptr);
    //Align tab, so it doesn't conflict
    //May want to reduce frequency of updates 
    if (!UpdateAlignStatus()) LOG_WARN("Fail Align Command");
    UpdateAlignErr();

    NewRaDec(currentRA, currentDEC);
    return true;
}


bool LX200_TeenAstro::SetTrackEnabled(bool enabled) //track On/Off events handled by inditelescope       Tested
{
    char response[RB_MAX_LEN];

    if (enabled)
    {
        if(!getCommandString(PortFD, response, ":Te#"))
        {
            LOGF_ERROR("===CMD==> Track On %s", response);
            return false;
        }
    }
    else
    {
    if(!getCommandString(PortFD, response, ":Td#"))
        {
            LOGF_ERROR("===CMD==> Track Off %s", response);
            return false;
        }
    }
    return true;
}

bool LX200_TeenAstro::setLocalDate(uint8_t days, uint8_t months, uint16_t years)
{
    years = years % 100;
    char cmd[32];

    snprintf(cmd, 32, ":SC%02d/%02d/%02d#", months, days, years);

    if (!sendTeenAstroCommand(cmd)) return true;
    return false;
}

bool LX200_TeenAstro::sendTeenAstroCommandBlind(const char *cmd)
{
    int error_type;
    int nbytes_write = 0;

    DEBUGF(DBG_SCOPE, "CMD <%s>", cmd);

    tcflush(PortFD, TCIFLUSH);

    if ((error_type = tty_write_string(PortFD, cmd, &nbytes_write)) != TTY_OK)
        return error_type;

    return 1;
}

bool LX200_TeenAstro::sendTeenAstroCommand(const char *cmd)
{
    char response[1];
    int error_type;
    int nbytes_write = 0, nbytes_read = 0;

    DEBUGF(DBG_SCOPE, "CMD <%s>", cmd);

    tcflush(PortFD, TCIFLUSH);

    if ((error_type = tty_write_string(PortFD, cmd, &nbytes_write)) != TTY_OK)
        return error_type;

    error_type = tty_read(PortFD, response, 1, TeenAstro_TIMEOUT, &nbytes_read);

    tcflush(PortFD, TCIFLUSH);

    if (nbytes_read < 1)
    {
        LOG_ERROR("Unable to parse response.");
        return error_type;
    }

    return (response[0] == '0');
}

bool LX200_TeenAstro::updateLocation(double latitude, double longitude, double elevation)
{
    INDI_UNUSED(elevation);

    if (isSimulation())
        return true;

    double TeenAstro_long = 360 - longitude ;
	while (TeenAstro_long < 0)
        TeenAstro_long += 360;
	while (TeenAstro_long > 360)
		TeenAstro_long -= 360;

    if (!isSimulation() && setSiteLongitude(PortFD, TeenAstro_long) < 0)
    {
        LOG_ERROR("Error setting site longitude coordinates");
        return false;
    }

    if (!isSimulation() && setSiteLatitude(PortFD, latitude) < 0)
    {
        LOG_ERROR("Error setting site latitude coordinates");
        return false;
    }

    char l[32]={0}, L[32]={0};
    fs_sexa(l, latitude, 3, 3600);
    fs_sexa(L, longitude, 4, 3600);

    LOGF_INFO("Site location updated to Lat %.32s - Long %.32s", l, L);

    return true;
}

int LX200_TeenAstro::setMaxElevationLimit(int fd, int max)   // According to standard command is :SoDD*#       Tested
{
    LOGF_INFO("<%s>", __FUNCTION__);

    char read_buffer[RB_MAX_LEN]={0};

    snprintf(read_buffer, sizeof(read_buffer), ":So%02d#", max);

    return (setStandardProcedure(fd, read_buffer));
}

int LX200_TeenAstro::setSiteLongitude(int fd, double Long)
{
    //DEBUGFDEVICE(lx200Name, DBG_SCOPE, "<%s>", __FUNCTION__);
    int d, m, s;
    char read_buffer[32];

    getSexComponents(Long, &d, &m, &s);

    snprintf(read_buffer, sizeof(read_buffer), ":Sg%.03d:%02d#", d, m);

    return (setStandardProcedure(fd, read_buffer));
}

// New, Multistar alignment goes here: 

IPState LX200_TeenAstro::AlignStartGeometric (int stars){
	//See here https://groups.io/g/TeenAstro/message/3624
	char cmd[8];

	LOG_INFO("Sending Command to Start Alignment");
	IUSaveText(&OSNAlignT[0],"Align STARTED");
	IUSaveText(&OSNAlignT[1],"GOTO a star, center it");
	IUSaveText(&OSNAlignT[2],"GOTO a star, Solve and Sync");
	IUSaveText(&OSNAlignT[3],"Press 'Issue Align'");
	IDSetText(&OSNAlignTP, "==>Align Started");
	// Check for max number of stars and gracefully fall back to max, if more are requested.
	char read_buffer[RB_MAX_LEN];
	if(getCommandString(PortFD, read_buffer, ":A?#"))
	{
		LOGF_INFO("Getting Max Star: response Error, response = %s>", read_buffer);
		return IPS_ALERT;
	}
	//Check max_stars
	int max_stars = read_buffer[0] - '0';
	if (stars > max_stars) {
		LOG_INFO("Tried to start Align with too many stars.");
		LOGF_INFO("Starting Align with %d stars", max_stars);
		stars = max_stars;
	}
	snprintf(cmd, sizeof(cmd), ":A%.1d#", stars);
	LOGF_INFO("Started Align with %s, max possible: %d", cmd, max_stars);
	sendTeenAstroCommandBlind(cmd);
	return IPS_BUSY;
}


IPState LX200_TeenAstro::AlignAddStar (){
	//See here https://groups.io/g/TeenAstro/message/3624
	char cmd[8];
	LOG_INFO("Sending Command to Record Star");
	strcpy(cmd, ":A+#");
	if(sendTeenAstroCommandBlind(cmd)) {
		return IPS_BUSY;
	}
	return IPS_ALERT;
}

bool LX200_TeenAstro::UpdateAlignStatus ()
{
	//  :A?#  Align status
	//         Returns: mno#
	//         where m is the maximum number of alignment stars
	//               n is the current alignment star (0 otherwise)
	//               o is the last required alignment star when an alignment is in progress (0 otherwise)
	
	char read_buffer[RB_MAX_LEN];
	char msg[40];
	char stars[5];
// 	IUFillText(&OSNAlignT[4], "4", "Current Status", "Not Updated");
// 	IUFillText(&OSNAlignT[5], "5", "Max Stars", "Not Updated");
// 	IUFillText(&OSNAlignT[6], "6", "Current Star", "Not Updated");
// 	IUFillText(&OSNAlignT[7], "7", "# of Align Stars", "Not Updated");
	
	int max_stars, current_star, align_stars;
//	LOG_INFO("Gettng Align Status");
	if(getCommandString(PortFD, read_buffer, ":A?#"))
	{
		LOGF_INFO("Align Status response Error, response = %s>", read_buffer);
		return false;
	}
// 	LOGF_INFO("Gettng Align Status: %s", read_buffer);
	max_stars = read_buffer[0] - '0';
	current_star = read_buffer[1] - '0';
	align_stars = read_buffer[2] - '0';
	snprintf(stars, sizeof(stars), "%d", max_stars);
	IUSaveText(&OSNAlignT[5],stars);
	snprintf(stars, sizeof(stars), "%d", current_star);
	IUSaveText(&OSNAlignT[6],stars);
	snprintf(stars, sizeof(stars), "%d", align_stars);
	IUSaveText(&OSNAlignT[7],stars);
	
	
/*	if (align_stars > max_stars) {
		LOGF_ERROR("Failed Sanity check, can't have more stars than max: :A?# gives: %s", read_buffer);
		return false;
	}*/
	
	if (current_star <= align_stars)
	{
		snprintf(msg, sizeof(msg), "%s Manual Align: Star %d/%d", read_buffer, current_star, align_stars );
		IUSaveText(&OSNAlignT[4],msg);
	}
	if (current_star > align_stars)
	{
		snprintf(msg, sizeof(msg), "Manual Align: Completed");
		IUSaveText(&OSNAlignT[4],msg);
		UpdateAlignErr();
	}
    IDSetText(&OSNAlignTP, nullptr);
	

	
	return true;
}

bool LX200_TeenAstro::UpdateAlignErr()
{
	//  :GXnn#   Get TeenAstro value
	//         Returns: value
	
	// 00 ax1Cor
	// 01 ax2Cor
	// 02 altCor
	// 03 azmCor
	// 04 doCor
	// 05 pdCor
	// 06 ffCor
	// 07 dfCor
	// 08 tfCor
	// 09 Number of stars, reset to first star
	// 0A Star  #n HA
	// 0B Star  #n Dec
	// 0C Mount #n HA
	// 0D Mount #n Dec
	// 0E Mount PierSide (and increment n)

	
	
	char read_buffer[RB_MAX_LEN];
    char polar_error[40], sexabuf[20];
    // 	IUFillText(&OSNAlignT[4], "4", "Current Status", "Not Updated");
    // 	IUFillText(&OSNAlignT[5], "5", "Max Stars", "Not Updated");
    // 	IUFillText(&OSNAlignT[6], "6", "Current Star", "Not Updated");
    // 	IUFillText(&OSNAlignT[7], "7", "# of Align Stars", "Not Updated");
	
	//	LOG_INFO("Gettng Align Error Status");
	if(getCommandString(PortFD, read_buffer, ":GX02#"))
	{
		LOGF_INFO("Polar Align Error Status response Error, response = %s>", read_buffer);
		return false;
	}
// 	LOGF_INFO("Getting Align Error Status: %s", read_buffer);
	
    long altCor = strtold(read_buffer, nullptr);
	if(getCommandString(PortFD, read_buffer, ":GX03#"))
	{
		LOGF_INFO("Polar Align Error Status response Error, response = %s>", read_buffer);
		return false;
	}
// 	LOGF_INFO("Getting Align Error Status: %s", read_buffer);
	
    long azmCor = strtold(read_buffer, nullptr);
    fs_sexa(sexabuf, (double)azmCor/3600, 4, 3600);
    snprintf(polar_error, sizeof(polar_error), "%ld' /%s", azmCor, sexabuf);
    IUSaveText(&OSNAlignErrT[1],polar_error);
    fs_sexa(sexabuf, (double)altCor/3600, 4, 3600);
    snprintf(polar_error, sizeof(polar_error), "%ld' /%s", altCor, sexabuf);
    IUSaveText(&OSNAlignErrT[0],polar_error);
    IDSetText(&OSNAlignErrTP, nullptr);
	
	
	return true;
}

IPState LX200_TeenAstro::AlignDone(){
	//See here https://groups.io/g/TeenAstro/message/3624
	char cmd[8];
	LOG_INFO("Sending Command to Finish Alignment and write");
	strcpy(cmd, ":AW#");
    IUSaveText(&OSNAlignT[0],"Align FINISHED Written to EEprom");
    IUSaveText(&OSNAlignT[1],"------");
    IUSaveText(&OSNAlignT[2],"------");
    IUSaveText(&OSNAlignT[3],"------");
    IDSetText(&OSNAlignTP, nullptr);
	if (sendTeenAstroCommandBlind(cmd)){
		return IPS_OK;
	}
	IUSaveText(&OSNAlignT[0],"Align WRITE FAILED");
    IDSetText(&OSNAlignTP, nullptr);
	return IPS_ALERT;
	
}
#ifdef TeenAstro_NOTDONE
IPState LX200_TeenAstro::OSEnableOutput(int output) {
	//  :SXnn,VVVVVV...#   Set TeenAstro value
	//          Return: 0 on failure
	//                  1 on success
	//	if (parameter[0]=='G') { // Gn: General purpose output
	// :SXGn,value 
	// value, 0 = low, other = high
	LOG_INFO("Not implemented yet");
	return IPS_OK;
}
#endif

IPState LX200_TeenAstro::OSDisableOutput(int output) {
	LOG_INFO("Not implemented yet");
	OSGetOutputState(output);
	return IPS_OK;
}

/*
bool LX200_TeenAstro::OSGetValue(char selection[2]) {
	//  :GXnn#   Get TeenAstro value
	//         Returns: value 
	//         Error = 123456789 
	//
	// Double unless noted: integer:i, special:* and values in {}
	// 
	//   00 ax1Cor
	//   01 ax2Cor
	//   02 altCor  //EQ Altitude Correction
	//   03 azmCor  //EQ Azimuth Correction
	//   04 doCor
	//   05 pdCor
	//   06 ffCor
	//   07 dfCor
	//   08 tfCor
	//   09 Number of stars, reset to first star
	//   0A Star  #n HA
	//   0B Star  #n Dec
	//   0C Mount #n HA
	//   0D Mount #n Dec
	//   0E Mount PierSide (and increment n)
	//   80 UTC time
	//   81 UTC date
	//   90 pulse-guide rate
	//   92 MaxRate
	//   93 MaxRate (default) number 
	// * 94 pierSide (N if never) {Same as :Gm# (E, W, None)}
	// i 95 autoMeridianFlip AutoFlip setting {0/1+}
	// * 96 preferred pier side {E, W, B}
	//   97 slew speed
	// * 98 rotator {D, R, N} 
	//   9A temperature in deg. C
	//   9B pressure in mb
	//   9C relative humidity in %
	//   9D altitude in meters
	//   9E dew point in deg. C
	//   9F internal MCU temperature in deg. C
	// * Un: Get stepper driver statUs
	//   En: Get settings
	//   Fn: Debug
	//   G0-GF (HEX!) = TeenAstro output status
	char value[64] ="  ";
	char command[64]=":$GXmm#";
	int error_type;
	command[4]=selection[0];
	command[5]=selection[1];
	//Should change to LOGF_DEBUG once tested
	LOGF_INFO("Command: %s", command);
	LOGF_INFO("Response: %s", command);
	if(getCommandString(PortFD, value, command) != TTY_OK) {
		return false;
		
}
*/

bool LX200_TeenAstro::OSGetOutputState(int output) {
	//  :GXnn#   Get TeenAstro value
	//         Returns: value
	// nn= G0-GF (HEX!) - Output status
	//
	char value[64] ="  ";
	char command[64]=":$GXGm#";
	LOGF_INFO("Output: %s", char(output));
	LOGF_INFO("Command: %s", command);
	command[5]=char(output);
	LOGF_INFO("Command: %s", command);
	getCommandString(PortFD, value, command);
	if (value[0] == 0) {
		OSOutput1S[0].s = ISS_ON;
		OSOutput1S[1].s = ISS_OFF;
	} else {
		OSOutput1S[0].s = ISS_OFF;
		OSOutput1S[1].s = ISS_ON;
	}
    IDSetSwitch(&OSOutput1SP, nullptr);
    return true;
}

bool LX200_TeenAstro::SetTrackRate(double raRate, double deRate) {
	char read_buffer[32];
	snprintf(read_buffer, sizeof(read_buffer), ":RA%04f#", raRate);
	LOGF_INFO("Setting: RA Rate to %04f", raRate);
	if (!sendTeenAstroCommand(read_buffer))
	{
		return false;
	}
	snprintf(read_buffer, sizeof(read_buffer), ":RE%04f#", deRate);
	LOGF_INFO("Setting: DE Rate to %04f", deRate);
	if (!sendTeenAstroCommand(read_buffer))
	{
		return false;
	}
	LOG_INFO("RA and DE Rates succesfully set");
	return true;
}
