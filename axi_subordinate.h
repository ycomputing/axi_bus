#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "axi_param.h"
#include "axi_bus.h"

SC_MODULE(AXI_SUBORDINATE)
{
	sc_fifo_in<axi_trans_t> request;
	sc_fifo_out<axi_trans_t> response;

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
	void write_memory_csv(const char* filename="s_memory_after.csv");
	void log(std::string source, std::string action, std::string detail);
};
