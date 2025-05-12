#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <furi_hal_gpio.h>

#include <gui/gui.h>
// #include <furi_hal_resources.h>
// #include <locale/locale.h>
// #include "si4735_app.h"
#include <furi_hal_i2c.h>

#include <notification/notification_messages.h>

#define TAG "si4735_device"

typedef enum {
    AM_MODE,
    __FM_MODE,
    __SSB_MODE,
    // SYNC_MODE,
    TOTAL_SWITCHING_MODES = 3,
} SwitchingModes;

typedef enum {
    KHz100,
    KHz1000,
    KHz10000,
    // SYNC_MODE,
    TOTAL_COEF_MODES = 3,
} CoefModes;

struct si4735App {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;

    // DrawMode draw_mode;
    SwitchingModes switching_mode;
    CoefModes coef_mode;

    const GpioPin* output_pin;
    const GpioPin* SHND_pin;
    const GpioPin* mute_pin;

    // bool input_value;
    bool mute_value;

    // FuriTimer* timer;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;

    NotificationApp* notifications;

    uint16_t freq_khz;
    uint8_t multiplier_freq;
    uint16_t offset;
    uint8_t snr;
    uint8_t rssi;
    uint8_t status;
    uint8_t n;
    uint16_t coef;

    uint8_t reciver_mode;

    uint8_t vol;

    uint16_t ID;
    char *PTy_buffer;
    char PSName[9];
    char rds_buffer2A[65];
};

/**
 * @ingroup group01
 *
 * @brief Block B data type
 *
 * @details For GCC on System-V ABI on 386-compatible (32-bit processors), the following stands:
 *
 * 1) Bit-fields are allocated from right to left (least to most significant).
 * 2) A bit-field must entirely reside in a storage unit appropriate for its declared type.
 *    Thus a bit-field never crosses its unit boundary.
 * 3) Bit-fields may share a storage unit with other struct/union members, including members that are not bit-fields.
 *    Of course, struct members occupy different parts of the storage unit.
 * 4) Unnamed bit-fields' types do not affect the alignment of a structure or union, although individual
 *    bit-fields' member offsets obey the alignment constraints.
 *
 * @see also Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0); pages 78 and 79
 * @see also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union
{
    struct
    {
        uint16_t address : 2;            // Depends on Group Type and Version codes. If 0A or 0B it is the Text Segment Address.
        uint16_t DI : 1;                 // Decoder Controll bit
        uint16_t MS : 1;                 // Music/Speech
        uint16_t TA : 1;                 // Traffic Announcement
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group0;
    struct
    {
        uint16_t address : 4;            // Depends on Group Type and Version codes. If 2A or 2B it is the Text Segment Address.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group2;
    struct
    {
        uint16_t content : 4;            // Depends on Group Type and Version codes.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } refined;
    struct
    {
        uint8_t lowValue;
        uint8_t highValue; // Most Significant byte first
    } raw;
} si47x_rds_blockb;

/**
 * @ingroup group01
 *
 * @brief Response data type for current channel and reads an entry from the RDS FIFO.
 *
 * @see Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0); pages 77 and 78
 */
typedef union
{
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1;
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        // RESP1
        uint8_t RDSRECV : 1;      //!<  RDS Received; 1 = FIFO filled to minimum number of groups set by RDSFIFOCNT.
        uint8_t RDSSYNCLOST : 1;  //!<  RDS Sync Lost; 1 = Lost RDS synchronization.
        uint8_t RDSSYNCFOUND : 1; //!<  RDS Sync Found; 1 = Found RDS synchronization.
        uint8_t DUMMY3 : 1;
        uint8_t RDSNEWBLOCKA : 1; //!<  RDS New Block A; 1 = Valid Block A data has been received.
        uint8_t RDSNEWBLOCKB : 1; //!<  RDS New Block B; 1 = Valid Block B data has been received.
        uint8_t DUMMY4 : 2;
        // RESP2
        uint8_t RDSSYNC : 1; //!<  RDS Sync; 1 = RDS currently synchronized.
        uint8_t DUMMY5 : 1;
        uint8_t GRPLOST : 1; //!<  Group Lost; 1 = One or more RDS groups discarded due to FIFO overrun.
        uint8_t DUMMY6 : 5;
        // RESP3 to RESP11
        uint8_t RDSFIFOUSED; //!<  RESP3 - RDS FIFO Used; Number of groups remaining in the RDS FIFO (0 if empty).
        uint8_t BLOCKAH;     //!<  RESP4 - RDS Block A; HIGH byte
        uint8_t BLOCKAL;     //!<  RESP5 - RDS Block A; LOW byte
        uint8_t BLOCKBH;     //!<  RESP6 - RDS Block B; HIGH byte
        uint8_t BLOCKBL;     //!<  RESP7 - RDS Block B; LOW byte
        uint8_t BLOCKCH;     //!<  RESP8 - RDS Block C; HIGH byte
        uint8_t BLOCKCL;     //!<  RESP9 - RDS Block C; LOW byte
        uint8_t BLOCKDH;     //!<  RESP10 - RDS Block D; HIGH byte
        uint8_t BLOCKDL;     //!<  RESP11 - RDS Block D; LOW byte
        // RESP12 - Blocks A to D Corrected Errors.
        // 0 = No errors;
        // 1 = 1–2 bit errors detected and corrected;
        // 2 = 3–5 bit errors detected and corrected.
        // 3 = Uncorrectable.
        uint8_t BLED : 2;
        uint8_t BLEC : 2;
        uint8_t BLEB : 2;
        uint8_t BLEA : 2;
    } resp;
    uint8_t raw[13];
} si47x_rds_status;

typedef struct si4735App si4735App;

// #define SI4734D60_RSTPORT GPIOA
// #define SI4734D60_RSTPIN GPIO7
// #define SI4734_RST_CLR() furi_hal_gpio_write(app->output_pin, 0); // gpio_clear(SI4734D60_RSTPORT, SI4734D60_RSTPIN)
// #define SI4734_RST_SET() furi_hal_gpio_write(app->output_pin, 1); // gpio_set(SI4734D60_RSTPORT, SI4734D60_RSTPIN)

/**********************************************************************
 *Простая библиотека для работы с si4734
 *4 мая 2021г
 *10 мая 2021 SI4734 поддерживает SSB с тем же патчем что и SI4735!!!
 *16 мая 2021 добавлен автопоиск станций в ам.
 **********************************************************************/

// #ifndef SI4734_H
// #define SI4734_H

#define SI4734I2C I2C1
#define SI4734ADR 0x11
#define SI4734D60_RSTPORT GPIOA
#define SI4734D60_RSTPIN GPIO7

#define POWER_UP 0x01       // Power up device and mode selection.
#define GET_REV 0x10        // Returns revision information on the device.
#define POWER_DOWN 0x11     // Power down device.
#define SET_PROPERTY 0x12   // Sets the value of a property.
#define GET_PROPERTY 0x13   // Retrieves a property’s value.
#define GET_INT_STATUS 0x14 // Read interrupt status bits.

// AM command
#define AM_TUNE_FREQ 0x40    // Tunes to a given AM frequency.
#define AM_SEEK_START 0x41   // Begins searching for a valid AM frequency.
#define AM_TUNE_STATUS 0x42  // Queries the status of the already issued AM_TUNE_FREQ or AM_SEEK_START command.
#define AM_RSQ_STATUS 0x43   // Queries the status of the Received Signal Quality (RSQ) for the current channel.
#define AM_AGC_STATUS 0x47   // Queries the current AGC settings.
#define AM_AGC_OVERRIDE 0x48 // Overrides AGC settings by disabling and forcing it to a fixed value.
#define GPIO_CTL 0x80        // Configures GPO1, 2, and 3 as output or Hi-Z.
#define GPIO_SET 0x81        // Sets GPO1, 2, and 3 output level (low or high).

// AM/SW/LW Receiver Property Summary
// See  Si47XX PROGRAMMING GUIDE AN332 (REV 1.0); page 125
#define DIGITAL_OUTPUT_FORMAT 0x0102                // Configure digital audio outputs.
#define DIGITAL_OUTPUT_SAMPLE_RATE 0x0104           // Configure digital audio output sample rate
#define REFCLK_FREQ 0x0201                          //Sets frequency of reference clock in Hz. The range is 31130 to 34406 Hz, or 0 to disable the AFC. Default is 32768 Hz.
#define REFCLK_PRESCALE 0x0202                      // Sets the prescaler value for RCLK input.
#define AM_DEEMPHASIS 0x3100                        // Sets deemphasis time constant. Can be set to 50 μs. Deemphasis is disabled by default.
#define AM_CHANNEL_FILTER 0x3102                    // Selects the bandwidth of the channel filter for AM reception. The choices are 6, 4, 3, 2, 2.5, 1.8, or 1 (kHz). The default bandwidth is 2 kHz.
#define AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN 0x3103 // Sets the maximum gain for automatic volume control.
#define AM_MODE_AFC_SW_PULL_IN_RANGE 0x3104         // Sets the SW AFC pull-in range.
#define AM_MODE_AFC_SW_LOCK_IN_RANGE 0x3105         // Sets the SW AFC lock-in.
#define AM_RSQ_INTERRUPTS 0x3200                    // Same SSB - Configures interrupt related to Received Signal Quality metrics. All interrupts are disabled by default.
#define AM_RSQ_SNR_HIGH_THRESHOLD 0x3201            //Sets high threshold for SNR interrupt.
#define AM_RSQ_SNR_LOW_THRESHOLD 0x3202             // Sets low threshold for SNR interrupt.
#define AM_RSQ_RSSI_HIGH_THRESHOLD 0x3203           // Sets high threshold for RSSI interrupt.
#define AM_RSQ_RSSI_LOW_THRESHOLD 0x3204            // Sets low threshold for RSSI interrupt.
#define AM_SOFT_MUTE_RATE 0x3300                    // Sets the attack and decay rates when entering or leaving soft mute. The default is 278 dB/s.
#define AM_SOFT_MUTE_SLOPE 0x3301                   // Sets the AM soft mute slope. Default value is a slope of 1.
#define AM_SOFT_MUTE_MAX_ATTENUATION 0x3302         // Sets maximum attenuation during soft mute (dB). Set to 0 to disable soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_SNR_THRESHOLD 0x3303           // Sets SNR threshold to engage soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_RELEASE_RATE 0x3304            // Sets softmute release rate. Smaller values provide slower release, and larger values provide faster release.
#define AM_SOFT_MUTE_ATTACK_RATE 0x3305             // Sets software attack rate. Smaller values provide slower attack, and larger values provide faster attack.
#define AM_SEEK_BAND_BOTTOM 0x3400                  // Sets the bottom of the AM band for seek. Default is 520.
#define AM_SEEK_BAND_TOP 0x3401                     // Sets the top of the AM band for seek. Default is 1710.
#define AM_SEEK_FREQ_SPACING 0x3402                 // Selects frequency spacing for AM seek. Default is 10 kHz spacing.
#define AM_SEEK_SNR_THRESHOLD 0x3403                // Sets the SNR threshold for a valid AM Seek/Tune.
#define AM_SEEK_RSSI_THRESHOLD 0x3404               // Sets the RSSI threshold for a valid AM Seek/Tune.
#define AM_AGC_ATTACK_RATE 0x3702                   // Sets the number of milliseconds the high peak detector must be exceeded before decreasing gain.
#define AM_AGC_RELEASE_RATE 0x3703                  // Sets the number of milliseconds the low peak detector must not be exceeded before increasing the gain.
#define AM_FRONTEND_AGC_CONTROL 0x3705              // Adjusts AM AGC for frontend (external) attenuator and LNA.
#define AM_NB_DETECT_THRESHOLD 0x3900               // Sets the threshold for detecting impulses in dB above the noise floor
#define AM_NB_INTERVAL 0x3901                       // Interval in micro-seconds that original samples are replaced by interpolated clean samples
#define AM_NB_RATE 0x3902                           // Noise blanking rate in 100 Hz units. Default value is 64.
#define AM_NB_IIR_FILTER 0x3903                     // Sets the bandwidth of the noise floor estimator. Default value is 300.
#define AM_NB_DELAY 0x3904                          // Delay in micro-seconds before applying impulse blanking to the original samples

#define RX_VOLUME    0x4000
#define RX_HARD_MUTE 0x4001


#define GPO_IEN 0x0001                       // AM and SSB - Enable interrupt source
#define SSB_BFO 0x0100                       // Sets the Beat Frequency Offset (BFO) under SSB mode.
#define SSB_MODE 0x0101                      // Sets number of properties of the SSB mode.
#define SSB_RSQ_INTERRUPTS 0x3200            // Configure Interrupts related to RSQ
#define SSB_RSQ_SNR_HI_THRESHOLD 0x3201      // Sets high threshold for SNR interrupt
#define SSB_RSQ_SNR_LO_THRESHOLD 0x3202      // Sets low threshold for SNR interrupt
#define SSB_RSQ_RSSI_HI_THRESHOLD 0x3203     // Sets high threshold for RSSI interrupt
#define SSB_RSQ_RSSI_LO_THRESHOLD 0x3204     // Sets low threshold for RSSI interrupt
#define SSB_SOFT_MUTE_RATE 0x3300            // Sets the attack and decay rates when entering or leaving soft mute
#define SSB_SOFT_MUTE_MAX_ATTENUATION 0x3302 // Sets the maximum attenuation during soft mute (db); 0dB to disable soft mute; defaul 8dB;
#define SSB_SOFT_MUTE_SNR_THRESHOLD 0x3303   // Sets SNR threshould to engage soft mute. Defaul 8dB
#define SSB_RF_AGC_ATTACK_RATE 0x3700        // Sets the number of milliseconds the high RF peak detector must be exceeded before decreasing the gain. Defaul 4.
#define SSB_RF_AGC_RELEASE_RATE 0x3701       // Sets the number of milliseconds the low RF peak detector must be exceeded before increasing the gain. Defaul 24.

// SSB
#define SSB_RF_IF_AGC_ATTACK_RATE 0x3702  // Sets the number of milliseconds the high IF peak detector must be exceeded before decreasing gain. Defaul 4.
#define SSB_RF_IF_AGC_RELEASE_RATE 0x3703 // Sets the number of milliseconds the low IF peak detector must be exceeded before increasing the gain. Defaul 140.

// See AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; pages 12 and 13
#define LSB_MODE 1 // 01
#define USB_MODE 2 // 10

// Parameters
#define SI473X_RDS_OUTPUT_ONLY 0 //0b00000000      // RDS output only (no audio outputs) Si4749 only
#define SI473X_ANALOG_AUDIO 5//0b00000101         // Analog Audio Inputs
#define SI473X_DIGITAL_AUDIO1 0x0b//0b00001011       // Digital audio output (DCLK, LOUT/DFS, ROUT/DIO)
#define SI473X_DIGITAL_AUDIO2 0xb0//0b10110000       // Digital audio outputs (DCLK, DFS, DIO)
#define SI473X_ANALOG_DIGITAL_AUDIO 0xb5//0b10110101 // Analog and digital audio outputs (LOUT/ROUT and DCLK, DFS,DIO)




#define FM_TUNE_FREQ 0x20
#define FM_SEEK_START 0x21 // Begins searching for a valid FM frequency.
#define FM_TUNE_STATUS 0x22
#define FM_AGC_STATUS 0x27
#define FM_AGC_OVERRIDE 0x28
#define FM_RSQ_STATUS 0x23
#define FM_RDS_STATUS 0x24 // Returns RDS information for current channel and reads an entry from the RDS FIFO.

// FM RDS properties
#define FM_RDS_INT_SOURCE 0x1500
#define FM_RDS_INT_FIFO_COUNT 0x1501
#define FM_RDS_CONFIG 0x1502
#define FM_RDS_CONFIDENCE 0x1503

#define FM_DEEMPHASIS 0x1100
#define FM_BLEND_STEREO_THRESHOLD 0x1105
#define FM_BLEND_MONO_THRESHOLD 0x1106
#define FM_BLEND_RSSI_STEREO_THRESHOLD 0x1800
#define FM_BLEND_RSSI_MONO_THRESHOLD 0x1801
#define FM_BLEND_SNR_STEREO_THRESHOLD 0x1804
#define FM_BLEND_SNR_MONO_THRESHOLD 0x1805
#define FM_BLEND_MULTIPATH_STEREO_THRESHOLD 0x1808
#define FM_BLEND_MULTIPATH_MONO_THRESHOLD 0x1809

// FM SEEK Properties
#define FM_SEEK_BAND_BOTTOM 0x1400         // Sets the bottom of the FM band for seek
#define FM_SEEK_BAND_TOP 0x1401            // Sets the top of the FM band for seek
#define FM_SEEK_FREQ_SPACING 0x1402        // Selects frequency spacing for FM seek
#define FM_SEEK_TUNE_SNR_THRESHOLD 0x1403  // Sets the SNR threshold for a valid FM Seek/Tune
#define FM_SEEK_TUNE_RSSI_THRESHOLD 0x1404 // Sets the RSSI threshold for a valid FM Seek/Tune

//-----------------------------------------------------------------------------
//группы RDS
#define RDS_ALL_GROUPTYPE_MASK 0xF000
#define RDS_ALL_GROUPTYPE_SHIFT 12
#define RDS_ALL_GROUPVER 0x800
#define RDS_ALL_TP 0x400
#define RDS_ALL_PTY_MASK 0x3E0
#define RDS_ALL_PTY_SHIFT 5
#define RDS_GROUP0_TA 4
#define RDS_GROUP0_MS 3
#define RDS_GROUP0_DI 2
#define RDS_GROUP0_C1C0_MASK 0x03 // (word)
#define RDS_GROUP4A_MJD15_16_MASK 0x03 // (word)
//#define RDS_GROUP4A_MJD0_14_MASK 0xFFFE
#define RDS_GROUP4A_MJD0_14_SHIFT 1
#define RDS_GROUP4A_HOURS4_MASK 0x01 // (word)
#define RDS_GROUP4A_HOURS0_3_MASK 0xF000
#define RDS_GROUP4A_HOURS0_3_SHIFT 12
#define RDS_GROUP4A_MINUTES_MASK 0x0FC0
#define RDS_GROUP4A_MINUTES_SHIFT 6
#define RDS_GROUP4A_LTO_SIGN_MASK 0x0020
#define RDS_GROUP4A_LTO_MASK 0x001F
#define RDS_GROUP2_ABFLAG_MASK 0x0010
#define RDS_GROUP2_ADDRESS_MASK 0x000F

#define RDSRECV_MASK 0x01
#define RDSSYNC_MASK 0x01
#define BLEA_MASK 0xC0
#define BLEB_MASK 0x30
#define BLEC_MASK 0x0C
#define BLED_MASK 0x03

void delay(uint16_t ms);
void si4734_reset(si4735App* app);
uint8_t si4734_am_mode();
uint8_t si4734_powerdown();
uint16_t si4734_get_prop(uint16_t prop);
uint8_t si4734_set_prop(uint16_t prop, uint16_t val);
uint8_t si4734_get_int_status();
void si4734_am_seek(uint16_t freq,uint8_t up);
uint8_t si4734_fm_mode();
uint8_t si4734_ssb_patch_mode(const uint8_t *patch);
uint8_t si4734_fm_set_freq(uint16_t freq_10khz);
uint8_t si4734_get_freq(uint16_t *freq,uint8_t *snr, uint8_t *rssi);
uint8_t si4734_ssb_signal_status(uint8_t *resp1,uint8_t *resp2,uint8_t *rssi,uint8_t *snr);
uint8_t si4734_fm_signal_status(uint8_t *rssi,uint8_t *snr,uint8_t *freq_of); // int8_t *freq_of
uint8_t si4734_get_rev(void);
uint8_t si4734_am_set_freq(uint16_t freq_khz);
uint8_t si4734_ssb_set_freq(uint16_t freq_khz);
void reciver_set_mode(si4735App* app, uint8_t rec_mod);
uint8_t si4735_RDS_set_interrupt();
uint8_t si4735_Configures_RDS_setting();
uint8_t si4735_RDS_set_group();
void si4734_volume(int8_t dv);
void show_freq(si4735App* app, uint16_t freq, int16_t offset);
uint8_t get_recivier_signal_status(si4735App* app, uint8_t *snr,uint8_t *rssi,uint8_t *freq_of);
uint8_t si4734_am_signal_status(uint8_t *resp1,uint8_t *resp2,uint8_t *rssi,uint8_t *snr);
void show_reciver_status(si4735App* app, uint8_t snr, uint8_t rssi, uint8_t status);
void show_reciver_full_status(si4735App* app, uint16_t freq, int16_t offset, uint8_t snr, uint8_t rssi, uint8_t status);
void show_RDS_hum_2(si4735App* app);
uint8_t get_recivier_RDS_status(si4735App* app, uint16_t *BLOCKA, uint16_t *BLOCKB, uint16_t *BLOCKC, uint16_t *BLOCKD, 
                                uint8_t *RDSFIFOUSED, uint8_t *RESP1, uint8_t *RESP2, uint8_t *RESP12);
uint8_t si4735_RDS_status(uint16_t *BLOCKA, uint16_t *BLOCKB, uint16_t *BLOCKC, uint16_t *BLOCKD, uint8_t *RDSFIFOUSED, uint8_t *RESP1, 
                            uint8_t *RESP2, uint8_t *RESP12);
void MJDDecode(unsigned long MJD, uint16_t * year, uint8_t * month, uint8_t * day);
void reciver_next_step(si4735App* app);
void getNext4Block(char *c, uint16_t *BLOCKC, uint16_t *BLOCKD);
// #endif
