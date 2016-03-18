#include "QRadioModuleSettings.h"

QRadioModuleSettings::QRadioModuleSettings(QObject *parent) :
    QObject(parent)
{
    RadioChanType = RADIOCHAN_TYPE_IDLE;
    RadioSignalPower = RADIO_SIGNALPOWER_LOW;
    ARMPowerMode = ARM_POWERMODE_NORMAL;

    NoTxFreqChan = DEFAULT_TX_FREQ_CHAN;
    NoRxFreqChan = DEFAULT_RX_FREQ_CHAN;

    AudioInLevel = DEFAULT_AUDIO_IN_GAIN;
    AudioOutLevel = DEFAULT_AUDIO_OUT_GAIN;

    RSSILevel = 0;

    RadioChanState = RADIOCHAN_STATE_IDLE;
}


uint8_t QRadioModuleSettings::ResetSettings()
{
    RadioChanType = RADIOCHAN_TYPE_IDLE;
    RadioSignalPower = RADIO_SIGNALPOWER_LOW;
    ARMPowerMode = ARM_POWERMODE_NORMAL;

    NoTxFreqChan = DEFAULT_TX_FREQ_CHAN;
    NoRxFreqChan = DEFAULT_RX_FREQ_CHAN;

    AudioInLevel = DEFAULT_AUDIO_IN_GAIN;
    AudioOutLevel = DEFAULT_AUDIO_OUT_GAIN;

    RSSILevel = 0;

    RadioChanState = RADIOCHAN_STATE_IDLE;

    return(0);
}

uint8_t QRadioModuleSettings::SetRadioChanType(uint8_t chanType)
{
    if(chanType<NUM_RADIOCHAN_TYPES)
    {
        RadioChanType	=	(en_RadioChanTypes)chanType;
        return(0);
    }
    else
        return(1);
}

uint8_t QRadioModuleSettings::GetRadioChanType()
{
    return(RadioChanType);
}

uint8_t QRadioModuleSettings::SetRadioSignalPower(uint8_t signalPower)
{
    if(signalPower<NUM_RADIO_SIGNALPOWER_MODES)
    {
        RadioSignalPower = (en_RadioSignalPowers)signalPower;
        return(0);
    }
    else
        return(1);
}

uint8_t QRadioModuleSettings::GetRadioSignalPower()
{
    return(RadioSignalPower);
}

uint8_t QRadioModuleSettings::SetARMPowerMode(uint8_t powerMode)
{
    if(powerMode<NUM_ARM_POWERMODES)
    {
        ARMPowerMode = (en_ARMPowerModes)powerMode;
        return(0);
    }
    else
        return(1);
}

uint8_t QRadioModuleSettings::GetARMPowerMode()
{
    return(ARMPowerMode);
}

uint8_t QRadioModuleSettings::SetTxFreqChan(uint16_t noFreqChan)
{
    NoTxFreqChan = noFreqChan;

    return(0);
}

uint16_t QRadioModuleSettings::GetTxFreqChan()
{
    return(NoTxFreqChan);
}

uint8_t QRadioModuleSettings::SetRxFreqChan(uint16_t noFreqChan)
{
    NoRxFreqChan = noFreqChan;

    return(0);
}

uint16_t QRadioModuleSettings::GetRxFreqChan()
{
    return(NoRxFreqChan);
}

uint8_t QRadioModuleSettings::SetAudioInLevel(uint8_t audioLevel)
{
    AudioInLevel = audioLevel;

    return(0);
}

uint8_t QRadioModuleSettings::GetAudioInLevel()
{
    return(AudioInLevel);
}

uint8_t QRadioModuleSettings::SetAudioOutLevel(uint8_t audioLevel)
{
    AudioOutLevel = audioLevel;

    return(0);
}

uint8_t QRadioModuleSettings::GetAudioOutLevel()
{
    return(AudioOutLevel);
}

uint8_t QRadioModuleSettings::GetRSSILevel()
{
    return(RSSILevel);
}

uint8_t QRadioModuleSettings::SetRSSILevel(uint8_t rssilLevel)
{
    RSSILevel = rssilLevel;

    return(0);
}

uint8_t QRadioModuleSettings::GetRadioChanState()
{
    return(RadioChanState);
}

uint8_t QRadioModuleSettings::SetRadioChanState(uint8_t radioChanState)
{
    RadioChanState = (en_RadioChanStates)radioChanState;

    return(0);
}
