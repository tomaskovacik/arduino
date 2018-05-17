//gnu gpl 3
//(C) T.Kovacik
// tomas.kovacik_AT_searchcompanyhere_DOT_com
//more info here
//how to connect here .... todo

#ifndef VAGFISReader_h
#define VAGFISReader_h

#include <inttypes.h>
#include <Arduino.h>
/*
volatile static uint8_t adr;
volatile static uint64_t msg1;
volatile static uint64_t msg2;
volatile static uint8_t cksum;
volatile static uint8_t newmsg1;
volatile static uint8_t newmsg2;
volatile static uint8_t adrok;
volatile static uint8_t cksumok;
volatile static uint8_t tmp_cksum;
volatile static int grabbing;*/
volatile static uint8_t msgbit;
static uint8_t newmsg_from_radio=0;
static uint8_t data[255];

static uint8_t FIS_READ_CLK;
static uint8_t FIS_READ_DATA;
static uint8_t FIS_READ_ENA;

class VAGFISReader
{

 public:
	VAGFISReader(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin);
	~VAGFISReader();
	void init();
//	uint64_t GetMsg(int msgid);
	uint8_t read_data(int8_t id);
	bool has_new_msg();
	void clear_new_msg_flag();

private:

/*static void read_cksum();
static void read_msg1();
static void read_msg2();
static void read_adr();*/
static void read_data_line();
static void detect_ena_line_rising();
static void detect_ena_line_falling();

};
#endif

