#ifndef __AXI_PARAM_H__
#define __AXI_PARAM_H__

// For detailed explanation, see AMBA AXI protocol spec
// https://developer.arm.com/documentation/ihi0022/latest/


#define ADDR_WIDTH	64
#define DATA_WIDTH	128

#define BUS_ACCESS_READ 'R'
#define BUS_ACCESS_WRITE 'W'

typedef sc_dt::sc_bigint<DATA_WIDTH> bus_data_t;

#define BUS_DATA_ZERO	0

// Maximum number of data with one address
// Too big value may cause program crash
#define AXI_TRANSACTION_LENGTH_MAX	100

// name of channels

#define CHANNEL_NAME_AW			"AW"
#define CHANNEL_NAME_W			"W"
#define CHANNEL_NAME_B			"B"
#define CHANNEL_NAME_AR			"AR"
#define CHANNEL_NAME_R			"R"
#define CHANNEL_NAME_READER		"reader"
#define CHANNEL_NAME_WRITER		"writer"
#define CHANNEL_NAME_MANAGER	"manager"

// channel id
#define CHANNEL_X		0
#define CHANNEL_AW		1
#define CHANNEL_W		2
#define CHANNEL_B		3
#define CHANNEL_AR		4
#define CHANNEL_R		5


// name of channel states

#define CHANNEL_ACCEPT		"ACCEPT"
#define CHANNEL_BUSY		"BUSY"
#define CHANNEL_BUSYQ		"BUSYQ"
#define CHANNEL_COMPLETE	"COMPLETE"
#define CHANNEL_ENQUEUE		"ENQUEUE"
#define CHANNEL_ERROR		"ERROR"
#define CHANNEL_HOLD		"HOLD"
#define CHANNEL_IDLE		"IDLE"
#define CHANNEL_INITIATE	"INITIATE"
#define CHANNEL_NO_MATCH	"NOMATCH"
#define CHANNEL_NOT_READY	"NOTREADY"
#define CHANNEL_OKAY		"OKAY"
#define CHANNEL_READ		"READ"
#define CHANNEL_READY		"READY"
#define CHANNEL_RECV		"RECV"
#define CHANNEL_RESPONSE	"RESPONSE"
#define CHANNEL_SEND		"SEND"	// send after idle
#define CHANNEL_SENDC		"SENDC"	// send after send
#define CHANNEL_SENDQ		"SENDQ"	// idle after send
#define CHANNEL_UNKNOWN		"UNKNOWN"
#define CHANNEL_WAITR		"WAITR"	// wait for READY
#define CHANNEL_WAITV		"WAITV"	// wait for VALID
#define CHANNEL_WRITE		"WRITE"

// unit of latency is in clock cycles
/*
#define LATENCY_READY_AW	10
#define LATENCY_READY_W		20
#define LATENCY_READY_B		3
#define LATENCY_READY_AR	10
#define LATENCY_READY_R		5
*/


#define LATENCY_READY_AW	0
#define LATENCY_READY_W		0
#define LATENCY_READY_B		0
#define LATENCY_READY_AR	0
#define LATENCY_READY_R		0


/*
#define LATENCY_READY_AW	100
#define LATENCY_READY_W		20
#define LATENCY_READY_B		3
#define LATENCY_READY_AR	100
#define LATENCY_READY_R		5
*/

// AW tuple: (AWID, AWADDR, AWLEN)
typedef std::tuple<uint32_t, uint64_t, uint8_t> tuple_AW_t;

// W tuple: (WID, WDATA, WLAST)
typedef std::tuple<uint32_t, bus_data_t, bool> tuple_W_t;

// AR tuple: (ARID, ARADDR, ARLEN)
typedef std::tuple<uint32_t, uint64_t, uint8_t> tuple_AR_t;

// R tuple: <RID, RDATA, RLAST>
typedef std::tuple<uint32_t, bus_data_t, bool> tuple_R_t;

// conversion utility functions

uint64_t address_from_hex_string(const std::string& str);
std::string address_to_hex_string(uint64_t address);
bus_data_t bus_data_from_hex_string(const std::string& str);
std::string bus_data_to_hex_string(bus_data_t data);

#endif