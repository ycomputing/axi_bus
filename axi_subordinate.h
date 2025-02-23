#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "axi_param.h"
#include "axi_bus.h"

struct when_trans_t
{
	uint64_t stamp;
	axi_trans_t trans;

	when_trans_t(uint64_t s, axi_trans_t t)
	{
		stamp = s;
		trans = t;
	}

	bool operator<(const when_trans_t t) const
	{
		// smaller stamp has higher priority
		return this->stamp > t.stamp;
	}
};

SC_MODULE(AXI_SUBORDINATE)
{
	sc_fifo_in<axi_trans_t> request;
	sc_fifo_out<axi_trans_t> response;

	std::mutex mutex_q;
	sc_event_queue event_something_to_send;
	std::priority_queue<when_trans_t> q_send;

	// pair<address, data>
	std::unordered_map<uint64_t, bus_data_t> map_memory;

	const char *filename_memory = "s_memory.csv";

	SC_CTOR(AXI_SUBORDINATE)
	{
		SC_THREAD(thread_reader);
		SC_THREAD(thread_writer);
		sensitive << event_something_to_send;
	}

	void thread_reader();
	void thread_writer();

	void fifo_reader();
	void fifo_writer();

	int get_latency_ns(axi_trans_t trans);

	void read_memory_csv();
	void write_memory_csv(const char* filename="s_memory_after.csv");
	void log(std::string source, std::string action, std::string detail);
};
