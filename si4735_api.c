#include "si4735_api.h"

#include "patch_init.h"

const char  PTy_0[] = "No program type or undefined";
const char  PTy_1[] = "News";
const char  PTy_2[] = "Current affairs";
const char  PTy_3[] = "Information";
const char  PTy_4[] = "Sport";
const char  PTy_5[] = "Education";
const char  PTy_6[] = "Drama";
const char  PTy_7[] = "Culture";
const char  PTy_8[] = "Science";
const char  PTy_9[] = "Varied";
const char PTy_10[] = "Pop music";
const char PTy_11[] = "Rock music";
const char PTy_12[] = "Easy listening";
const char PTy_13[] = "Light classical";
const char PTy_14[] = "Serious classical";
const char PTy_15[] = "Other music";
const char PTy_16[] = "Weather";
const char PTy_17[] = "Finance";
const char PTy_18[] = "Children’s programs";
const char PTy_19[] = "Social affairs";
const char PTy_20[] = "Religion";
const char PTy_21[] = "Phone-in";
const char PTy_22[] = "Travel";
const char PTy_23[] = "Leisure";
const char PTy_24[] = "Jazz music";
const char PTy_25[] = "Country music";
const char PTy_26[] = "National music";
const char PTy_27[] = "Oldies music";
const char PTy_28[] = "Folk music";
const char PTy_29[] = "Documentary";
const char PTy_30[] = "Alarm test";
const char PTy_31[] = "Alarm";

const char* const PTyList[] = {PTy_0,  PTy_1,  PTy_2,  PTy_3,  PTy_4,  PTy_5,  PTy_6,  PTy_7,
                               PTy_8,  PTy_9,  PTy_10, PTy_11, PTy_12, PTy_13, PTy_14, PTy_15,
                               PTy_16, PTy_17, PTy_18, PTy_19, PTy_20, PTy_21, PTy_22, PTy_23,
                               PTy_24, PTy_25, PTy_26, PTy_27, PTy_28, PTy_29, PTy_30, PTy_31};


uint8_t PTy = 255;
char PSName[9]; // Значение PSName
char PSName_prev[9];
uint8_t PSNameUpdated = 0; // Для отслеживания изменений в PSName
//-----------------------------------------------------------------------------

uint16_t MaybeThisIDIsReal = 0;
uint8_t IDRepeatCounter = 0;
#define REPEATS_TO_BE_REAL_ID 3
uint16_t ID = 0;
bool ID_printed = false;
static bool PTy_printed = false;

#define _AM_MODE 0
#define _FM_MODE 1
#define _SSB_MODE 2
// #define SYNC_MODE 3

uint16_t MIN_LIMIT=200;
uint16_t  MAX_LIMIT=30000;
//#define IF_FEQ 455

uint16_t encoder=15200;
uint16_t pwm1=750;
uint16_t coef=5;
uint8_t encoder_mode=2;
int16_t bfo=0;
int16_t vol=0x32;  // 0x1a

/*
11 метров, 25.60 — 26.10 МГц (11,72 — 11,49 метра).
13 метров, 21.45 — 21.85 МГц (13,99 — 13,73 метра).
15 метров, 18.90 — 19.02 МГц (15,87 — 15,77 метра).
16 метров, 17.55 — 18.05 МГц (17,16 — 16,76 метра).
19 метров, 15.10 — 15.60 МГц (19,87 — 18,87 метра).
22 метра, 13.50 — 13.87 МГц (22,22 — 21,63 метра).
25 метров 11.60 — 12.10 МГц (25,86 — 24,79 метра).
31 метр, 9.40 — 9.99 МГц (31,91 — 30,03 метра).
41 метр, 7.20 — 7.50 МГц (41,67 — 39,47 метра).
49 метров, 5.85 — 6.35 МГц (52,36 — 47,66 метра).
60 метров, 4.75 — 5.06 МГц (63,16 — 59,29 метра).
75 метров, 3.90 — 4.00 МГц (76,92 — 75 метров).
90 метров, 3.20 — 3.40 МГц (93,75 — 88,24 метров).
120 метров (средние волны), 2.30 — 2.495 МГц (130,43 — 120,24 метра).
*/
uint16_t bands[]={200,1000,3100,3600,5800,7200,9300,11200,13500,14200,15100,17450,21500,27000};
uint8_t steps[]={1,5,10,50};
uint8_t reciver_mode=0;
//0 - am, 1 -fm, 2 - ssb

si47x_rds_status currentRdsStatus;       //!<  current RDS status

int rdsTextAdress2A; //!<  rds_buffer2A current position

char rds_buffer2A[65]; //!<  RDS Radio Text buffer - Program Information
char rds_buffer2A_prev[65];

void delay(uint16_t ms){
	uint64_t temp;
	temp=ms<<10;
	while(temp--)__asm__("nop");
}

void si4734_reset(si4735App* app){
	furi_hal_gpio_write(app->output_pin, false); // SI4734_RST_CLR();
	delay(10); // delay(10); // furi_delay_ms(10);
	furi_hal_gpio_write(app->output_pin, true); // SI4734_RST_SET();
	delay(10); // delay(10); // furi_delay_ms(10);
}

uint8_t si4734_fm_mode(){
	FURI_LOG_E(TAG, "si4734_fm_mode()");
	// ARG1 (1<<4)|0 AN322 p130
	// ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x10,0x05};
	uint8_t status=0, tray=0;
	uint32_t timeout = 100;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	// bool status_b = false;
    // status_b = furi_hal_i2c_tx(&furi_hal_i2c_handle_external, SI4734ADR, cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x10, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x05, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	furi_delay_ms(1000); // furi_delay_ms(1000); // delay(1000);
#if 1	
    do{ 
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255){
			FURI_LOG_E(TAG, "tray==255");
            furi_hal_i2c_release(&furi_hal_i2c_handle_external);
            return 0xff;
        }
		delay(50); // furi_delay_ms(50); // delay(50);
	}while(status!=0x80);
#endif
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status; // status;
}

uint8_t si4734_am_mode(){
	// ARG1 (1<<4)|1 AN322 p130
	// ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x11,0x05};
	uint8_t status, tray=0;
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	furi_delay_ms(1000); // delay(1000);
	do{ 
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
	}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_set_prop(uint16_t prop, uint16_t val){
	uint8_t cmd[6]={SET_PROPERTY,0,(prop>>8),
									(prop&0xff),(val>>8),(val&0xff)};
	uint8_t status;
	uint32_t timeout = 100;
	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
	
	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(100); // furi_delay_ms(100);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
	
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

void reciver_set_mode(si4735App* app, uint8_t rec_mod){
	static uint16_t amfreq=602,fmfreq=9920, ssbfreq=4996;//запоминаем старое значение // 8910
	
	si4734_powerdown();
								//частоты
	// if(app->reciver_mode==_FM_MODE)fmfreq=app->freq_khz; else amfreq=app->freq_khz;
	if(rec_mod==_AM_MODE){
		//o_printf("AM mode\n");
		app->reciver_mode=_AM_MODE;
		si4734_am_mode();
		si4734_set_prop(AM_CHANNEL_FILTER, 0x0100);
		si4734_set_prop(AM_SOFT_MUTE_MAX_ATTENUATION, 0);//soft mute off
		si4734_set_prop(AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN, 0x5000); //60дб
		si4734_set_prop(RX_VOLUME, app->vol);
		//si4734_set_prop(AM_SEEK_BAND_TOP, 30000);
		MIN_LIMIT=200;
		MAX_LIMIT=30000;
		//encoder=15200;
		// app->freq_khz=amfreq-bfo/1000;//поправка на bfo // encoder=
		app->freq_khz=amfreq;
		bfo=bfo%1000;
		FURI_LOG_I(TAG, "freq_khz:%d\r", app->freq_khz);
		si4734_am_set_freq(app->freq_khz); // encoder
		coef=1;
		app->coef=coef;
		encoder_mode=0;
	} else if(rec_mod==_FM_MODE){
		//oled_clear();
		//o_printf("FM mode\n");
		app->reciver_mode=_FM_MODE;
		si4734_fm_mode();
		si4734_set_prop(FM_DEEMPHASIS,0x0001);//01 = 50 µs. Used in Europe, Australia, Japan
		si4734_set_prop(RX_VOLUME, app->vol);
		MIN_LIMIT=6000;
		MAX_LIMIT=11100;
		coef=1; // coef=1;
		app->coef=coef;
		//encoder=8910;
		// encoder=fmfreq;
		app->freq_khz = fmfreq;
		si4734_fm_set_freq(app->freq_khz); // encoder
		encoder_mode=0;
		//-----RDS-----
		// uint8_t status = 0;
		// char buff[14];
		si4735_RDS_set_interrupt();
		// sprintf(buff, "status = %c\r\n", status);
		// usart_transmit(&tx_rb, buff);
		si4735_RDS_set_group();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);
		si4735_Configures_RDS_setting();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);

		// status = si4734_get_int_status();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);
	}else{
		app->reciver_mode=_SSB_MODE;
		//bfo=0;
		si4734_ssb_patch_mode(ssb_patch_content);
		si4734_set_prop(0x0101,((1<<15)|(1<<12)|(1<<4)|2));//ssb man page 24
		si4734_set_prop(SSB_BFO, bfo);
		si4734_set_prop(AM_SOFT_MUTE_MAX_ATTENUATION, 0);//soft mute off
		si4734_set_prop(AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN, 0x7000); //84дб
		si4734_set_prop(RX_VOLUME, app->vol);
		MIN_LIMIT=200;
		MAX_LIMIT=30000;
		//encoder=7100;
		app->freq_khz=ssbfreq; // app->freq_khz // encoder
		si4734_ssb_set_freq(app->freq_khz);
		coef=1;
		app->coef=coef;
		encoder_mode=0;
	}
}

uint8_t si4735_RDS_set_interrupt(){
	uint8_t status=0;
	/*
	  Enable RDSRECV interrupt (set RDSINT bit when RDS has filled the
	  FIFO by the amount set on FM_RDS_INTERRUPT_FIFO_COUNT
	  Reply Status. Clear-to-send high
	*/
	status = si4734_set_prop(0x1500, 0x0001); // FM_RDS_INT_SOURCE
	// usart_transmit(&tx_rb, "RDS_interrupt_setup: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4735_RDS_set_group(){
	uint8_t status=0;
	/*
	  Sets the minimum number of
	  RDS groups stored in the
	  receive FIFO required before
	  RDSRECV is set.
	*/
	status = si4734_set_prop(0x1501, 0x0001); // FM_RDS_INT_FIFO_COUNT // 0x0004
	// usart_transmit(&tx_rb, "RDS_set_group: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4735_Configures_RDS_setting(){
	uint8_t status=0;
	/*
	  Configures RDS setting.
	*/
	status = si4734_set_prop(0x1502, 0xFF01); // FM_RDS_CONFIG // 0xEF01
	// usart_transmit(&tx_rb, "Configures_RDS_setting: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4734_ssb_patch_mode(const uint8_t *patch){
	//ARG1 (1<<4)|1 AN322 p130
	//ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x31,0x05};
	uint8_t status, tray=0;
	uint16_t count,iterate=0;
	uint32_t timeout = 1000;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	delay(1000);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
		}while(status!=0x80);
	if(status!=0x80) return 0x1;
	//count=(sizeof patch)/8;
	count=0x451;
	while(count--){
		tray=0;
		furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), patch+iterate, 8, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,patch+iterate,8,0,0);
		iterate+=8;
		delay(2);
		do{	
			furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
			tray++;
			if(tray==255){
				furi_hal_i2c_release(&furi_hal_i2c_handle_external);
				return 0x02;
			} 
			delay(1);
		}while(status!=0x80);
	}
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);
	return status;
}

uint8_t si4734_fm_set_freq(uint16_t freq_10khz){
	// ID_printed = false;
	PTy_printed = false;
	uint8_t fast,freq_h,freq_l,status,tray=0;
	fast=0;
	freq_h=freq_10khz>>8;
	freq_l=freq_10khz&0xff;
	uint8_t cmd[6]={FM_TUNE_FREQ,fast,freq_h,freq_l,0,0};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255){
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_am_set_freq(uint16_t freq_khz){
	uint8_t fast,freq_h,freq_l,status,tray=0;
	fast=0;
	freq_h=freq_khz>>8;
	freq_l=freq_khz&0xff;
	uint8_t cmd[6]={AM_TUNE_FREQ,fast,freq_h,freq_l,0,1};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_ssb_set_freq(uint16_t freq_khz){
	uint8_t mode,freq_h,freq_l,status,tray=0;
	if(freq_khz>10000)mode=0b10000000;else mode=0b01000000;
	freq_h=freq_khz>>8;
	freq_l=freq_khz&0xff;
	uint8_t cmd[6]={AM_TUNE_FREQ,mode,freq_h,freq_l,0,1};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_powerdown(){
	uint8_t cmd=POWER_DOWN,status;
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), &cmd, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,&cmd,1,0,0);
	//usart_transmit("test3!\r\n");
	delay(200);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
	// usart_transmit(&tx_rb, "si4734_powerdown: COMPLITED\r\n");
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);
	return status;
}

void si4734_volume(int8_t dv){
	int16_t vol=si4734_get_prop(0x4000);//AN332 p170
	vol+=dv;
	if(vol<0)vol=0;
	if(vol>0x3f)vol=0x3f;
	si4734_set_prop(0x4000,vol);
}

uint16_t si4734_get_prop(uint16_t prop){
	uint8_t cmd[4]={GET_PROPERTY,0,(prop>>8),(prop&0xff)};
	uint8_t answer[4];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 4, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,4,0,0);
	delay(100);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 4, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,4);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return (answer[2]<<8)|answer[3];
}

void show_freq(si4735App* app, uint16_t freq, int16_t offset){
	//NB колонки в пиксилях, а строки по 8 пикселей, отсчёт с нуля из
	//верхнего левого угла
	uint16_t offset_hz;
	UNUSED(offset_hz);
	uint16_t freq_khz;
	// char buff[30];
	if(app->reciver_mode==_FM_MODE){
		// o_printf_at(18*5,1,1,0,"x10");
		app->multiplier_freq = 10;
		// sprintf(buff, "x10\r\n");
		// FURI_LOG_I(TAG, "x10");
	}
	else{ 
		app->multiplier_freq = 1;
		// o_printf_at(18*5,1,1,0,"   ");
		// sprintf(buff, "   \r\n");
	}
	// usart_transmit(&tx_rb, buff);
	//Вся эта канитель от того что bfo работает не как описанно в 
	//в даташите f=f-bfo а f=f-bfo
	if(app->reciver_mode==_SSB_MODE){
		offset_hz=(1000-offset%1000)%1000;
		freq_khz=freq-offset/1000;
		if(offset%1000>0)freq_khz--;
		// o_printf_at(18*5,3,1,0,"%03d",offset_hz);
		// sprintf(buff, "%03d\r\n", offset_hz);
		// usart_transmit(&tx_rb, buff);
		// o_printf_at(0,1,3,0,"%5d",freq_khz);
		app->freq_khz=freq_khz;
		// sprintf(buff, "%5d\r\n", freq_khz);
		// usart_transmit(&tx_rb, buff);
	}
	else{
		// o_printf_at(18*5,3,1,0,"   ");
		// sprintf(buff, "   \r\n");
		// usart_transmit(&tx_rb, buff);
		// o_printf_at(0,1,3,0,"%5d",freq);
		app->freq_khz=freq;
		// sprintf(buff, "%5d\r\n", freq);
		// usart_transmit(&tx_rb, buff);
		// FURI_LOG_I(TAG, "%5d", freq);
		}

	// o_printf_at(18*5,2,1,0,"KHz");
	// sprintf(buff, "KHz\r\n");
	// usart_transmit(&tx_rb, buff);
	
}

uint8_t get_recivier_signal_status(si4735App* app, uint8_t *snr,uint8_t *rssi,uint8_t *freq_of){
	uint8_t status,resp1,resp2;
	switch(app->reciver_mode){
		case _AM_MODE: status=si4734_am_signal_status(&resp1,&resp2,rssi,snr);
			break;
		case _FM_MODE: status=si4734_fm_signal_status(rssi,snr,freq_of);
			break;
		case _SSB_MODE: status=si4734_ssb_signal_status(&resp1,&resp2,rssi,snr); // si4734_ssb_signal_status // si4734_am_signal_status(&resp1,&resp2,rssi,snr)
			break;
		default:
			status=0xff;
			break;
		}
	return status;
}

uint8_t si4734_ssb_signal_status(uint8_t *resp1,uint8_t *resp2,uint8_t *rssi,uint8_t *snr){
	uint8_t cmd[3]={AM_RSQ_STATUS,0x1};
	uint8_t tray=0;
	uint8_t answer[6];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);
	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,6);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
		}
	*resp1=answer[1];
	*resp2=answer[2];
	*rssi=answer[4];
	*snr=answer[5];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
}

uint8_t si4734_fm_signal_status(uint8_t *rssi,uint8_t *snr,uint8_t *freq_of){
	uint8_t cmd[3]={FM_RSQ_STATUS,0x1};
	uint8_t tray=0;
	uint8_t answer[8];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);
	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 8, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,8);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
	}

	*rssi=answer[4];
	*snr=answer[5];
	*freq_of=answer[7];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
	
}

uint8_t si4734_am_signal_status(uint8_t *resp1,uint8_t *resp2,uint8_t *rssi,uint8_t *snr){
	uint8_t cmd[3]={AM_RSQ_STATUS,0x1};
	uint8_t tray=0;
	uint8_t answer[6];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);

	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,6);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
		}
	*resp1=answer[1];
	*resp2=answer[2];
	*rssi=answer[4];
	*snr=answer[5];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
	}

void show_reciver_status(si4735App* app, uint8_t snr, uint8_t rssi, uint8_t status){
	uint8_t n=1;
	//o_printf_at(1,4,1,0,"SNR:%2ddB SI: %2duVdB",snr,rssi);
	app->snr=snr;
	app->rssi=rssi;
	// char buff[30];
	// sprintf(buff, "SNR:%2ddB SI: %2duVdB\r\n",snr,rssi);
	// usart_transmit(&tx_rb, buff);
	//coef - глобальная переменная
	//n поправочный коэфициэнт шага
	if(app->reciver_mode==_FM_MODE)n=10;
	app->n=n;
	//o_printf_at(1,5,1,0,"status x%x %dKHz   ",status,coef*n);
	app->status=status;
	// sprintf(buff, "status x%x %dKHz   \r\n",status,coef*n);
	// usart_transmit(&tx_rb, buff);
}	

void show_reciver_full_status(si4735App* app, uint16_t freq, int16_t offset,uint8_t snr, uint8_t rssi, uint8_t status){
	show_freq(app, freq, offset);
	show_reciver_status(app, snr,rssi,status);
}

uint8_t get_recivier_RDS_status(si4735App* app, uint16_t *BLOCKA, uint16_t *BLOCKB, uint16_t *BLOCKC, uint16_t *BLOCKD, 
								uint8_t *RDSFIFOUSED, uint8_t *RESP1, uint8_t *RESP2, uint8_t *RESP12){
	uint8_t status=0xff; // ,resp1,resp2
	switch(app->reciver_mode){
		case _AM_MODE: 
			// status=si4734_am_signal_status(&resp1,&resp2,rssi,snr);
			break;
		case _FM_MODE: 
			// status=si4734_fm_signal_status(rssi,snr,freq_of);
			status=si4735_RDS_status(BLOCKA,BLOCKB,BLOCKC,BLOCKD,RDSFIFOUSED,RESP1,RESP2,RESP12);
			break;
		case _SSB_MODE: 
			// status=si4734_am_signal_status(&resp1,&resp2,rssi,snr);
			break;
		default:
			status=0xff;
			break;
		}
	return status;
}

uint8_t si4735_RDS_status(uint16_t *BLOCKA, uint16_t *BLOCKB, uint16_t *BLOCKC, uint16_t *BLOCKD, uint8_t *RDSFIFOUSED, uint8_t *RESP1, uint8_t *RESP2,
						   uint8_t *RESP12){
	uint8_t cmd[3]={0x24,0x1};
	uint8_t tray=0;
	uint8_t answer[13];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);
	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 13, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,13);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
	}
	/*
		int val = ADCL + (ADCH << 8);
	 */
	*BLOCKA=answer[5] + (answer[4] << 8);
	*BLOCKB=answer[7] + (answer[6] << 8);
	*BLOCKC=answer[9] + (answer[8] << 8);
	*BLOCKD=answer[11] + (answer[10] << 8);

	*RDSFIFOUSED=answer[3];

	*RESP1=answer[1];

	*RESP2=answer[2];

	*RESP12=answer[12];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
	
}

void MJDDecode(unsigned long MJD, uint16_t * year, uint8_t * month, uint8_t * day){
  #if 0
	unsigned long L = 2400000 + MJD + 68570;
	unsigned long N = (L * 4) / 146097;
	L = L - (146097.0 * N + 3) / 4;
	(*year) = 4000 * (L + 1) / 1461001;
	L = L - 1461 * (*year) / 4 + 31;
	(*month) = 80.0 * L / 2447.0;
	(*day) = L - 2447 * (*month) / 80;
	L = (*month) / 11;
	(*month) = (*month) + 2 - 12 * L;
	(*year) = 100 * (N - 49) + year + L;
  #endif
    // Добавляем смещение для перехода к юлианской дате
    int jd = MJD + 2400001;

    // Переменные для промежуточных вычислений
    int A, B, C, D, E;

    // Преобразование JD в григорианскую дату
    A = jd + 32044;
    B = (4 * A + 3) / 146097;
    C = A - (146097 * B) / 4;
    D = (4 * C + 3) / 1461;
    E = C - (1461 * D) / 4;
    int monthDay = (5 * E + 2) / 153;

    *day = E - (153 * monthDay + 2) / 5 + 1;
    *month = monthDay + 3 - 12 * (monthDay / 10);
    *year = 100 * B + D - 4800 + (monthDay / 10);
}

/**
 * @ingroup group16 RDS status 
 * 
 * @brief Process data received from group 2A
 * 
 * @param c  char array reference to the "group  2A" text 
 */
void getNext4Block(char *c, uint16_t *BLOCKC, uint16_t *BLOCKD)
{
    // c[0] = currentRdsStatus.resp.BLOCKCH;
    // c[1] = currentRdsStatus.resp.BLOCKCL;
    // c[2] = currentRdsStatus.resp.BLOCKDH;
    // c[3] = currentRdsStatus.resp.BLOCKDL;

	c[0] = *BLOCKC >> 8;
    c[1] = *BLOCKC;
    c[2] = *BLOCKD >> 8;
    c[3] = *BLOCKD;

}

#if 1
void show_RDS_hum_2(si4735App* app){ // uint16_t BLOCKA, int16_t BLOCKB, uint16_t BLOCKC, uint16_t BLOCKD
	// UNUSED(app);
	uint8_t errLevelA, errLevelB, errLevelC, errLevelD, groupType;
	UNUSED(errLevelA);
	UNUSED(errLevelB);
	UNUSED(errLevelC);
	UNUSED(errLevelD);
  	bool groupVer;

	// char buff[30];
	// uint16_t BLOCKA, BLOCKB, BLOCKC, BLOCKD;
	// get_recivier_RDS_status(&BLOCKA, &BLOCKB, &BLOCKC, &BLOCKD);
	// print_RDS();
	uint16_t BLOCKA, BLOCKB, BLOCKC, BLOCKD;
	uint8_t status,RDSFIFOUSED,RESP1,RESP2,RESP12;
	UNUSED(status);
	//-----добавил на втором этапе--------------------------------------
	si47x_rds_blockb blkB;
	//------------------------------------------------------------------
	status = get_recivier_RDS_status(app, &BLOCKA, &BLOCKB, &BLOCKC, &BLOCKD, &RDSFIFOUSED, &RESP1, &RESP2, &RESP12);
	if(RESP1&RDSRECV_MASK){
		if(RESP2&RDSSYNC_MASK && RDSFIFOUSED > 0){
			if (BLOCKA == MaybeThisIDIsReal) {
				if (IDRepeatCounter < REPEATS_TO_BE_REAL_ID) {
					IDRepeatCounter++; // Значения совпадают, отразим это в счетчике
					if (IDRepeatCounter == REPEATS_TO_BE_REAL_ID)
						ID = MaybeThisIDIsReal; // Определились с ID станции
				}
			}
			else {
				IDRepeatCounter = 0; // Значения не совпадают, считаем заново
				MaybeThisIDIsReal = BLOCKA;
			}
			if (ID == 0) return; // Пока не определимся с ID, разбирать RDS не будем
			if (BLOCKA != ID) return; // ID не совпадает. Пропустим эту RDS группу
			// ID станции не скачет, вероятность корректности группы в целом выше
			if (!ID_printed) { // Выведем ID
				// Serial.print("ID: ");
				// Serial.println(ID, HEX);
				
				// sprintf(buff, "ID: %X\r\n", ID);
				// usart_transmit(&tx_rb, buff);

				// app->ID=ID;

				ID_printed = true; // Установим флаг чтобы больше не выводить ID
			}
			if((RESP12&(BLEB_MASK<3))||true){ // с проверкой на ошибку не работает, у меня ошибки или неправильно запрограммировал?
				// Блок B корректный, можем определить тип и версию группы
				// status = get_recivier_RDS_status(&BLOCKA, &BLOCKB, &BLOCKC, &BLOCKD); // 1
				if (!PTy_printed) { // Но сначала считаем PTy
					if (PTy == (BLOCKB & RDS_ALL_PTY_MASK) >> RDS_ALL_PTY_SHIFT) { 
						// Считаем PTy корректным, выведем его
						char *PTy_buffer = (char*) malloc(30);
						// strcpy_P(PTy_buffer, (char*)pgm_read_word(&(PTyList[PTy])));
						strcpy(PTy_buffer, PTyList[PTy]);
						// Serial.print("PTy: ");
						
						// usart_transmit(&tx_rb, "PTy: ");
						// Serial.println(PTy_buffer);
						// usart_transmit(&tx_rb, PTy_buffer);
						// usart_transmit(&tx_rb, "\r\n");
						
						// app->PTy_buffer=PTy_buffer; // запись ввобще неверна for(int i=0;i<size;++i){app->PTy_buffer[i]=PTy_buffer}
						strcpy(app->PTy_buffer, PTy_buffer); // здесь ловится NULL pointer

						free(PTy_buffer);
						PTy_printed = true;
					}
					else PTy = (BLOCKB & RDS_ALL_PTY_MASK) >> RDS_ALL_PTY_SHIFT;
				}
				groupType = (BLOCKB & RDS_ALL_GROUPTYPE_MASK) >> RDS_ALL_GROUPTYPE_SHIFT;
				groupVer = (BLOCKB & RDS_ALL_GROUPVER) > 0;
				// ************* 0A, 0B - PSName, PTY ************
				if ((groupType == 0)) { //if((groupType == 0) and (errLevelD < 3))
					//blockD = getRegister(RDA5807M_REG_BLOCK_D);
					// status = get_recivier_RDS_status(&BLOCKA, &BLOCKB, &BLOCKC, &BLOCKD); // 1
					// Сравним новые символы PSName со старыми:
					char c = (uint8_t)(BLOCKD >> 8); // новый символ // char c = uint8_t(BLOCKD >> 8); // новый символ
					uint8_t i = (BLOCKB & (uint16_t)RDS_GROUP0_C1C0_MASK) << 1; // его позиция в PSName
					if (PSName[i] != c) { // символы различаются
						PSNameUpdated &= !((1 << i) != 0); // сбросим флаг в PSNameUpdated // здесь может быть необходимо ==0
						PSName[i] = c;
					}
					else // символы совпадают, установим флаг в PSNameUpdated:
						PSNameUpdated |= 1 << i;
					// Аналогично для второго символа
					c = (uint8_t)(BLOCKD & 255); // c = uint8_t(BLOCKD & 255);
					i++;
					if (PSName[i] != c) {
						PSNameUpdated &= !((1 << i)!=0); // здесь может быть необходимо ==0
						PSName[i] = c;
					}
					else
						PSNameUpdated |= (1 << i); // здесь может быть надо !=0
					// Когда все 8 флагов в PSNameUpdated установлены, считаем что PSName получено полностью
					if (PSNameUpdated == 255) {
						// Дополнительное сравнение с предыдущим значением, чтобы не дублировать в Serial
						if (strcmp(PSName, PSName_prev) != 0) {
							
							//Serial.print("PSName: ");
							// usart_transmit(&tx_rb, "PSName: ");
							//Serial.println(PSName);
							// usart_transmit(&tx_rb, PSName);
							// usart_transmit(&tx_rb, "\r\n");
							// for(uint8_t i=0;i<9;++i){
								// app->PSName[i]=PSName[i];
							// }
							strcpy(app->PSName, PSName);
							
							strcpy(PSName_prev, PSName);
						}
					}
				} // PSName, PTy end
				// ******************************************
				// ******** 2A - Gets the Text processed for the 2A group ********
				/**
 				 * @ingroup group16 RDS status 
 				 * 
 				 * @brief Gets the Text processed for the 2A group
 				 * 
 				 * @return char* string with the Text of the group A2  
 				 */
				if ((groupType == 2) /*&& (groupVer == 0)*/ /* && getRdsVersionCode() == 0 */) {
					// Process group 2A
            		// Decode B block information
            		// blkB.raw.highValue = currentRdsStatus.resp.BLOCKBH;
            		// blkB.raw.lowValue = currentRdsStatus.resp.BLOCKBL;
					blkB.raw.highValue = BLOCKB >> 8;
            		blkB.raw.lowValue = BLOCKB;
            		rdsTextAdress2A = blkB.group2.address;

					if (rdsTextAdress2A >= 0 && rdsTextAdress2A < 16)
					{
						getNext4Block(&rds_buffer2A[rdsTextAdress2A * 4], &BLOCKC, &BLOCKD);
						rds_buffer2A[63] = '\0';
						// return rds_buffer2A;
						strcpy(app->rds_buffer2A, rds_buffer2A);
					#if 0	
						if (strcmp(rds_buffer2A, rds_buffer2A_prev) != 0) {
							strcpy(app->rds_buffer2A, rds_buffer2A);
							strcpy(rds_buffer2A_prev, rds_buffer2A);
						}
					#endif
					}
				}
				// ******************************************
				// ******** 4A - Clock time and date ********
				if ((groupType == 4) && (groupVer == 0)) { // (groupType == 4) and (groupVer == 0) and (errLevelC < 3) and (errLevelD < 3)
					// blockC = getRegister(RDA5807M_REG_BLOCK_C);
					// blockD = getRegister(RDA5807M_REG_BLOCK_D);
					// status = get_recivier_RDS_status(&BLOCKA, &BLOCKB, &BLOCKC, &BLOCKD); // 1
					// char buf[30];

					unsigned long MJD;
					uint16_t year;
					uint8_t month, day;
					MJD = (BLOCKB & RDS_GROUP4A_MJD15_16_MASK);
					MJD = (MJD << 15) | (BLOCKC >> RDS_GROUP4A_MJD0_14_SHIFT);
					// Serial.print("Date: ");
					// usart_transmit(&tx_rb, "Date: "); // вывожу
					if ((MJD < 58844) || (MJD > 62497)){ 
						// Serial.println("decode error");
						// usart_transmit(&tx_rb, "decode error\r\n"); // вывожу
					}
					else {
						MJDDecode(MJD, &year, &month, &day);
						if ((day <=31) && (month <= 12)) {
							// sprintf(buf, "%02d.%02d.%04d\r\n", day, month, year); // вывожу
							// Serial.println(buf);
							// usart_transmit(&tx_rb, buf); // вывожу
						}
						else{
							// Serial.println("decode error");
							// usart_transmit(&tx_rb, "decode error\r\n"); // вывожу
						}
					}
				
					long timeInMinutes;
					uint8_t hours, minutes, LTO;
					UNUSED(LTO);
					hours = (BLOCKC & RDS_GROUP4A_HOURS4_MASK) << 4;
					hours |= (BLOCKD & RDS_GROUP4A_HOURS0_3_MASK) >> RDS_GROUP4A_HOURS0_3_SHIFT;
					minutes = (BLOCKD & RDS_GROUP4A_MINUTES_MASK) >> RDS_GROUP4A_MINUTES_SHIFT;
					if ((hours > 23) || (minutes > 59)){
						// Serial.println("Time: decode error");
						// usart_transmit(&tx_rb, "Time: decode error\r\n"); // вывожу
					}
					else {
						timeInMinutes = hours * 60 + minutes;
						LTO = BLOCKD & RDS_GROUP4A_LTO_MASK;
						if (BLOCKD & RDS_GROUP4A_LTO_SIGN_MASK) {
							timeInMinutes -= (BLOCKD & RDS_GROUP4A_LTO_MASK) * 30;
							if (timeInMinutes < 0) timeInMinutes += 60 * 24;
						}  
						else {
							timeInMinutes += (BLOCKD & RDS_GROUP4A_LTO_MASK) * 30;
							if (timeInMinutes > 60 * 24) timeInMinutes -= 60 * 24;
						}
						hours = timeInMinutes / 60;
						minutes = timeInMinutes % 60;
						// sprintf(buf, "Time: %02d:%02d\r\n", hours, minutes); // вывожу
						// Serial.println(buf);
						// usart_transmit(&tx_rb, buf); // вывожу
					}
				} // Clock end
				// ******************************************
			}
		}
		// After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);
	}		

}
#endif

void reciver_next_step(si4735App* app){
	uint8_t st=app->n;
	st*=10;
	if(st>200)st=1;
	app->n=st;
}