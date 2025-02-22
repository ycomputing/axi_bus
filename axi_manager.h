#include <systemc>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>

#include "axi_param.h"
#include "axi_bus.h"

SC_MODULE(AXI_MANAGER)
{
	sc_fifo_out<axi_trans_t> request;
	sc_fifo_in<axi_trans_t> response;

	const char *filename_access = "m_access.csv";

	// pair<address, data>
	std::unordered_map<uint64_t, bus_data_t> map_memory;

	// queue access tuple: (timestamp, access_type(r/w), address, length, data)
	std::queue<std::tuple<uint64_t, axi_trans_t>> queue_access;

	SC_CTOR(AXI_MANAGER)
	{
		SC_THREAD(thread_sender);
		SC_THREAD(thread_receiver);
	}

	void thread_sender();
	void thread_receiver();
	void fifo_sender();
	void fifo_receiver();

	void log(std::string source, std::string action, std::string detail);

	uint32_t generate_transaction_id();

	void read_access_csv();
	void write_memory_csv(const char* filename="m_memory_after.csv");
};
