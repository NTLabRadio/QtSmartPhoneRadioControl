#ifndef QRADIOMODULESETTINGS_H
#define QRADIOMODULESETTINGS_H

#include <QObject>
#include <stdint.h>

class QRadioModuleSettings : public QObject
{
    Q_OBJECT
public:
    explicit QRadioModuleSettings(QObject *parent = 0);

    uint8_t ResetSettings();

    uint8_t SetRadioChanType(uint8_t chanType);
    uint8_t GetRadioChanType();

    uint8_t SetRadioSignalPower(uint8_t signalPower);
    uint8_t GetRadioSignalPower();

    uint8_t SetARMPowerMode(uint8_t powerMode);
    uint8_t GetARMPowerMode();

    uint8_t SetTxFreqChan(uint16_t noFreqChan);
    uint16_t GetTxFreqChan();

    uint8_t SetRxFreqChan(uint16_t noFreqChan);
    uint16_t GetRxFreqChan();

    uint8_t SetAudioInLevel(uint8_t audioLevel);
    uint8_t GetAudioInLevel();

    uint8_t SetAudioOutLevel(uint8_t audioLevel);
    uint8_t GetAudioOutLevel();

    uint8_t SetRSSILevel( uint8_t rssilLevel);
    uint8_t GetRSSILevel();

    uint8_t SetRadioChanState(uint8_t radioChanState);
    uint8_t GetRadioChanState();

    enum en_RadioChanTypes
    {
        RADIOCHAN_TYPE_IDLE,
        RADIOCHAN_TYPE_VOICE		=1,
        RADIOCHAN_TYPE_DATA		=2,
        RADIOCHAN_TYPE_RTKDATA	=3,
        NUM_RADIOCHAN_TYPES
    };

    enum en_RadioSignalPowers
    {
        RADIO_SIGNALPOWER_LOW       =0,
        RADIO_SIGNALPOWER_HIGH      =1,
        NUM_RADIO_SIGNALPOWER_MODES
    };

    enum en_ARMPowerModes
    {
        ARM_POWERMODE_NORMAL       =0,
        ARM_POWERMODE_SLEEP           =1,
        NUM_ARM_POWERMODES
    };

    enum en_RadioChanStates
    {
        RADIOCHAN_STATE_IDLE,
        RADIOCHAN_STATE_RECEIVE,
        RADIOCHAN_STATE_WAIT_RECEIVE,
        RADIOCHAN_STATE_TRANSMIT
    };

    enum en_RadioModuleStates
    {
        RADIOMODULE_STATE_IDLE,
        RADIOMODULE_STATE_TX_WAITING,
        RADIOMODULE_STATE_TX_RUNNING,
        RADIOMODULE_STATE_RX_WAITING,
        RADIOMODULE_STATE_RX_RUNNING,
        NUM_RADIOMODULE_STATES
    };

    static const uint32_t RADIO_BASE_FREQ_KHZ           = 410000;
    static const uint32_t RADIO_FREQCHAN_STEP_KHZ   = 25;

signals:

public slots:

private:
    //Тип радиоканала
    en_RadioChanTypes RadioChanType;
    //Режим мощности передатчика
    en_RadioSignalPowers	RadioSignalPower;
    //Режим энергосбережения
    en_ARMPowerModes ARMPowerMode;

    //Рабочие частоты передачи и приема
    uint16_t NoTxFreqChan;
    uint16_t NoRxFreqChan;

    uint8_t AudioInLevel;
    uint8_t AudioOutLevel;

    uint8_t RSSILevel;
    en_RadioChanStates RadioChanState;

    struct RadioModuleSettings
    {
        en_RadioChanTypes radioChanType;
        en_RadioSignalPowers	radioSignalPower;
        en_ARMPowerModes powerMode;

        uint16_t noTxFreqChan;
        uint16_t noRxFreqChan;

        uint8_t audioInLevel;
        uint8_t audioOutLevel;

        uint8_t levelRSSI;

        en_RadioChanStates radioChanState;
    };

    static const uint8_t DEFAULT_AUDIO_IN_GAIN 	= 5;
    static const uint8_t DEFAULT_AUDIO_OUT_GAIN = 7;

    static const uint16_t DEFAULT_TX_FREQ_CHAN	= 960;
    static const uint16_t DEFAULT_RX_FREQ_CHAN	= 960;
};

#endif // QRADIOMODULESETTINGS_H
