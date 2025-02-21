#ifndef __AXI_BUS_H__
#define __AXI_BUS_H__

#include <systemc>
#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <functional>

#include "axi_param.h"

typedef struct struct_axi_trans
{
	uint64_t	addr;
	uint8_t		length;
	bus_data_t	data[AXI_TRANSACTION_LENGTH_MAX];
	bool		is_write;

} axi_trans_t;

// The following function is required by 6.23.3 of IEEE std 1666-2011
inline std::ostream& operator<<(std::ostream& os, const struct_axi_trans& trans)
{
	return os;
}


typedef std::tuple<axi_trans_t, uint8_t> tuple_progress_t;

typedef struct
{
	int			channel;
	uint32_t	id;
	uint64_t	addr;
	uint8_t		len; // length - 1
	bus_data_t	data;
	bool		is_last;
} axi_bus_info_t;

SC_MODULE(AXI_BUS)
{
	sc_in<bool>	ACLK;
	sc_in<bool>	ARESETn;

	sc_fifo_in<axi_trans_t> request_M;
	sc_fifo_out<axi_trans_t> response_M;
	sc_fifo_in<axi_trans_t> response_S;
	sc_fifo_out<axi_trans_t> request_S;

	// Chapter A2.1.1 write request channel
	sc_signal<bool>			AWVALID;
	sc_signal<bool>			AWREADY;
	sc_signal<uint32_t>		AWID;
	sc_signal<uint64_t>		AWADDR;
	sc_signal<uint8_t>		AWLEN;

	// Chapter A2.1.2 write data channel
	sc_signal<bool>			WVALID;
	sc_signal<bool>			WREADY;
	sc_signal<uint32_t>		WID;
	sc_signal<bus_data_t>	WDATA;
	sc_signal<bool>			WLAST;

	// Chapter A2.1.3 write response channel
	sc_signal<bool>			BVALID;
	sc_signal<bool>			BREADY;
	sc_signal<uint32_t>		BID;

	// Chapter A2.2.1 read request channel
	sc_signal<bool>			ARVALID;
	sc_signal<bool>			ARREADY;
	sc_signal<uint32_t>		ARID;
	sc_signal<uint64_t>		ARADDR;
	sc_signal<uint8_t>		ARLEN;

	// Chapter A2.2.2 read data channel
	sc_signal<bool>			RVALID;
	sc_signal<bool>			RREADY;
	sc_signal<uint32_t>		RID;
	sc_signal<bus_data_t>	RDATA;
	sc_signal<bool>			RLAST;

	std::queue<axi_bus_info_t> q_send_AW;
	std::queue<axi_bus_info_t> q_send_W;
	std::queue<axi_bus_info_t> q_send_B;
	std::queue<axi_bus_info_t> q_send_AR;
	std::queue<axi_bus_info_t> q_send_R;

	std::queue<axi_bus_info_t> q_recv_AW;
	std::queue<axi_bus_info_t> q_recv_W;
	std::queue<axi_bus_info_t> q_recv_B;
	std::queue<axi_bus_info_t> q_recv_AR;
	std::queue<axi_bus_info_t> q_recv_R;

	std::unordered_map<uint32_t, tuple_progress_t> map_progress;

	SC_CTOR(AXI_BUS)
	{
		SC_CTHREAD(thread, ACLK);
		async_reset_signal_is(ARESETn, false);
	}

	void thread();
	void on_clock();
	void on_reset();

	axi_bus_info_t create_null_info();
	void send_info(int channel, axi_bus_info_t& info);
	axi_bus_info_t recv_info(int channel);

	static std::string get_channel_name(int channel);
	
	bool is_ready(int channel);
	bool is_valid(int channel);
	void set_ready(int channel, bool value);
	void set_valid(int channel, bool value);

	void channel_transaction();
	void fifo_transaction();

	static std::string transaction_to_string(const axi_trans_t& trans);
	static std::string bus_info_to_string(const axi_bus_info_t& info);
	std::string progress_to_string(const tuple_progress_t& progress);

	void transaction_request_M();
	void transaction_response_S();
	void transaction_response_M();
	void transaction_request_S();
	bool transaction_send_q(sc_fifo_out<axi_trans_t>& fifo_out, std::queue<axi_bus_info_t>& q);

	bool progress_create(axi_bus_info_t& info, bool is_write);
	void progress_delete(axi_bus_info_t& info);
	bool progress_update(std::queue<axi_bus_info_t>& q);

	void channel_sender(int channel, std::queue<axi_bus_info_t>& q);
	void channel_receiver(int channel, std::queue<axi_bus_info_t>& q);

	void log(int channel, std::string action, std::string detail);
	static void log(std::string source, std::string action, std::string detail);

	uint32_t generate_transaction_id();

	void progress_dump();
};

#endif