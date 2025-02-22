#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "axi_param.h"
#include "axi_bus.h"

// latency in clock period.
#define AXI_SUBORDINATE_LATENCY  20

SC_MODULE(AXI_SUBORDINATE)
{
	sc_fifo_in<axi_trans_t> request;
	sc_fifo_out<axi_trans_t> response;

	bool has_pending_response;
	axi_trans_t trans_pending;

	// pair<address, data>
	std::unordered_map<uint64_t, bus_data_t> map_memory;

	const char *filename_memory = "s_memory.csv";

	SC_CTOR(AXI_SUBORDINATE)
	{
		SC_THREAD(thread);
	}

	void thread();

	void fifo_manager();

	void read_memory_csv();
	void write_memory_csv(const char* filename);
	void log(std::string source, std::string action, std::string detail);
};
