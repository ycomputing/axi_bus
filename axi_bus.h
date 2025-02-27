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
#include <mutex>
#include <unordered_map>
#include "axi_param.h"
#include "axi_outstanding.h"

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

	// To access many queues from many threads, we need to use mutex
	std::mutex mutex_q;
	sc_event_queue event_something_to_send;
	sc_event_queue event_outstanding_R_has_room;
	sc_event_queue event_outstanding_W_has_room;

	// outstanding management
	axi_outstanding outstanding_R;
	axi_outstanding outstanding_W;

	SC_CTOR(AXI_BUS)
	{
		outstanding_R.name = "R";
		outstanding_R.is_write = false;
		outstanding_W.name = "W";
		outstanding_W.is_write = true;
		
		SC_THREAD(thread_clock);
		sensitive << ACLK << ARESETn;
		SC_THREAD(thread_request_M);
		SC_THREAD(thread_response_M);
		sensitive << event_something_to_send;
		SC_THREAD(thread_request_S);
		sensitive << event_something_to_send;
		SC_THREAD(thread_response_S);
	}

	void on_clock();
	void on_reset();

	void thread_clock();
	void thread_request_M();
	void thread_response_M();
	void thread_request_S();
	void thread_response_S();

	axi_bus_info_t create_null_info();
	void send_info(int channel, axi_bus_info_t& info);
	axi_bus_info_t recv_info(int channel);

	static std::string get_channel_name(int channel);
	
	bool is_address(int channel);
	bool is_ready(int channel);
	bool is_valid(int channel);
	void set_ready(int channel, bool value);
	void set_valid(int channel, bool value);
	bool is_recv_full(int channel);

	void channel_transaction();

	void transaction_request_M();
	void transaction_response_S();
	void transaction_response_M();
	void transaction_request_S();
	std::string transaction_send_info(sc_fifo_out<axi_trans_t>& fifo_out, axi_bus_info_t& info);

	bool outstanding_create(axi_bus_info_t& info, bool is_write);
	void outstanding_delete(axi_bus_info_t& info);
	bool outstanding_update(std::queue<axi_bus_info_t>& q);
	bool outstanding_is_present(uint32_t id);

	void channel_sender(int channel, std::queue<axi_bus_info_t>& q);
	void channel_receiver(int channel, std::queue<axi_bus_info_t>& q);

	void log(int channel, std::string action, std::string detail);
	static void log(std::string source, std::string action, std::string detail);

	uint32_t generate_transaction_id();
	void wait_enough_delta_cycles();

	void outstanding_dump();
};

#endif