#ifndef __AXI_PARAM_H__
#define __AXI_PARAM_H__

// For detailed explanation, see AMBA AXI protocol spec
// https://developer.arm.com/documentation/ihi0022/latest/

#define ADDR_WIDTH	64
#define DATA_WIDTH	128

#define BUS_ACCESS_READ 'R'
#define BUS_ACCESS_WRITE 'W'

typedef sc_dt::sc_bigint<DATA_WIDTH> bus_data_t;

// Maximum number of data with one address
// AxLEN width is 8 bits, so biggest possible is 256
#define AXI_TRANSACTION_LENGTH_MAX	256

// channel queue size
#define AXI_BUS_Q_SIZE_RECV	2
#define AXI_BUS_Q_SIZE_SEND 2

// maximum number of entries in the progress
// how many outstanding transactions can be
#define AXI_BUS_OUTSTANDING_MAX	2

// channel id
#define CHANNEL_AW		1
#define CHANNEL_W		2
#define CHANNEL_B		3
#define CHANNEL_AR		4
#define CHANNEL_R		5

// name of channel states

#define CHANNEL_HOLD			"HOLD"
#define CHANNEL_IDLE			"IDLE"
#define CHANNEL_NOT_READY		"NOT_READY"
#define CHANNEL_READY			"READY"
#define CHANNEL_READY_Q			"READY_Q"
#define CHANNEL_RECV			"RECV"
#define CHANNEL_RECV_FULL		"RECV_FULL"
#define CHANNEL_RECV_CONTINUE	"RECV_CONTINUE"
#define CHANNEL_SEND		"SEND"
#define CHANNEL_SENDC		"SENDC"	// send after send
#define CHANNEL_UNKNOWN		"UNKNOWN"
#define CHANNEL_WAIT_R		"WAIT_R"	// wait for READY
#define CHANNEL_WAIT_V		"WAIT_V"	// wait for VALID


// common structures

typedef struct struct_axi_trans
{
	uint64_t	addr;
	uint8_t		length;
	bus_data_t	data[AXI_TRANSACTION_LENGTH_MAX];
	bool		is_write;
	uint8_t		progress;
} axi_trans_t;

// The following function is required by 6.23.3 of IEEE std 1666-2011
inline std::ostream& operator<<(std::ostream& os, const struct_axi_trans& trans)
{
	return os;
}

typedef struct
{
	uint32_t	id;
	uint64_t	addr;
	uint8_t		len; // length - 1
	bus_data_t	data;
	bool		is_last;
} axi_bus_info_t;

// conversion utility functions

uint64_t address_from_hex_string(const std::string& str);
std::string address_to_hex_string(uint64_t address);
bus_data_t bus_data_from_hex_string(const std::string& str);
std::string bus_data_to_hex_string(const bus_data_t& data);

std::string bus_info_to_string(const axi_bus_info_t& info);
std::string transaction_to_string(const  axi_trans_t& trans);

#endif
