/*
    LX200 TeenAstro
    based on LX200 OnStep azwing (alain@zwingelstein.org)
    Contributors:
    James Lan https://github.com/james-lan
    Ray Wells https://github.com/blueshawk

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

    ===========================================
    Version 1.0: Start after 1.6 OnStep driver

*/

#pragma once

#include "lx200generic.h"
#include "lx200driver.h"
#include "indicom.h"

#include <cstring>
#include <unistd.h>
#include <termios.h>

#define RB_MAX_LEN 64

#define setParkTeenAstro(fd)  write(fd, "#:hQ#", 5)
#define ReticPlus(fd)      write(fd, "#:B+#", 5)
#define ReticMoins(fd)     write(fd, "#:B-#", 5)
#define TeenAstroalign1(fd)   write(fd, "#:A1#", 5)
#define TeenAstroalign2(fd)   write(fd, "#:A2#", 5)
#define TeenAstroalign3(fd)   write(fd, "#:A3#", 5)
#define TeenAstroalignOK(fd)   write(fd, "#:A+#", 5)
#define TeenAstro
#define RB_MAX_LEN 64

enum Errors {ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM, ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC, ERR_PARK, ERR_GOTO_SYNC};

class LX200_TeenAstro : public LX200Generic
{
  public:
    LX200_TeenAstro();
    ~LX200_TeenAstro() {}

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual void ISGetProperties(const char *dev) override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

  protected:
    virtual void getBasicData() override;
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool SetCurrentPark() override;
    virtual bool SetDefaultPark() override;
    virtual bool SetTrackEnabled(bool enabled) override;
    virtual bool updateLocation(double latitude, double longitude, double elevation) override;
    virtual bool setLocalDate(uint8_t days, uint8_t months, uint16_t years) override;
    virtual bool ReadScopeStatus() override;
    virtual int setSiteLongitude(int fd, double Long);
    virtual bool SetTrackRate(double raRate, double deRate) override;


    //Outputs
    IPState OSEnableOutput(int output);
    IPState OSDisableOutput(int output);
    bool OSGetOutputState(int output);


    bool sendTeenAstroCommand(const char *cmd);
    bool sendTeenAstroCommandBlind(const char *cmd);
    int  setMaxElevationLimit(int fd, int max);

    ITextVectorProperty ObjectInfoTP;
    IText ObjectInfoT[1] {};

    ISwitchVectorProperty StarCatalogSP;
    ISwitch StarCatalogS[3];

    ISwitchVectorProperty DeepSkyCatalogSP;
    ISwitch DeepSkyCatalogS[7];

    ISwitchVectorProperty SolarSP;
    ISwitch SolarS[10];

    INumberVectorProperty ObjectNoNP;
    INumber ObjectNoN[1];

    INumberVectorProperty MaxSlewRateNP;
    INumber MaxSlewRateN[2];

    INumberVectorProperty BacklashNP;    //test
    INumber BacklashN[2];    //Test

    INumberVectorProperty ElevationLimitNP;
    INumber ElevationLimitN[2];

    ITextVectorProperty VersionTP;
    IText VersionT[5] {};

    // TeenAstro Status controls
    ITextVectorProperty TeenAstroStatTP;
    IText TeenAstroStat[10] {};


    int IsTracking = 0;

    // Reticle +/- Buttons
    ISwitchVectorProperty ReticSP;
    ISwitch ReticS[2];

    // Align Buttons
    ISwitchVectorProperty TrackCompSP;
    ISwitch TrackCompS[3];

    ISwitchVectorProperty FrequencyAdjustSP;
    ISwitch FrequencyAdjustS[3];

    ISwitchVectorProperty AutoFlipSP;
    ISwitch AutoFlipS[2];

    ISwitchVectorProperty HomePauseSP;
    ISwitch HomePauseS[3];

    ISwitchVectorProperty SetHomeSP;
    ISwitch SetHomeS[2];

    ISwitchVectorProperty PreferredPierSideSP;
    ISwitch PreferredPierSideS[3];

    INumberVectorProperty minutesPastMeridianNP;
    INumber minutesPastMeridianN[2];


    ISwitchVectorProperty OSNAlignStarsSP;
    ISwitch OSNAlignStarsS[7];
    ISwitchVectorProperty OSNAlignSP;
    ISwitch OSNAlignS[4];
    IText OSNAlignT[8] {};
    ITextVectorProperty OSNAlignTP;
    IText OSNAlignErrT[4] {};
    ITextVectorProperty OSNAlignErrTP;
    char OSNAlignStat[RB_MAX_LEN];

    ISwitchVectorProperty OSOutput1SP;
    ISwitch OSOutput1S[2];
    ISwitchVectorProperty OSOutput2SP;
    ISwitch OSOutput2S[2];

    char OSStat[RB_MAX_LEN];
    char OldOSStat[RB_MAX_LEN];


    char OSPier[RB_MAX_LEN];
    char OldOSPier[RB_MAX_LEN];

  private:
    int currentCatalog;
    int currentSubCatalog;
    bool FirstRead=true;
};
