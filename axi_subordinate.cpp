#include <algorithm>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <map>

using namespace sc_core;
using namespace sc_dt;

#include "axi_subordinate.h"

void AXI_SUBORDINATE::thread()
{
	on_reset();
	wait();

	while(true)
	{
		on_clock();
		wait();
	}
}

void AXI_SUBORDINATE::on_clock()
{
	fifo_manager();
}

void AXI_SUBORDINATE::on_reset()
{
	has_pending_response = false;
}

void AXI_SUBORDINATE::fifo_manager()
{
	std::string log_action;
	std::string log_detail;

	bool is_accepted;
	axi_trans_t trans;

	if (has_pending_response)
	{
		is_accepted = response.nb_write(trans_pending);
		if (is_accepted)
		{
			has_pending_response = false;
			log(__FUNCTION__, "SENT_PENDING", AXI_BUS::transaction_to_string(trans_pending));
		}
		else
		{
			// still pending. do not receive incoming requests
			// try to send the response again on next clock.
			log(__FUNCTION__, "STILL_PENDING", AXI_BUS::transaction_to_string(trans_pending));
			return;
		}
	}

	// No pending response, now can receive incoming requests.
	is_accepted = request.nb_read(trans);
	if (is_accepted == false)
	{
		// No request now
		log(__FUNCTION__, "NO_REQUEST", "");
		return;
	}

	log(__FUNCTION__, "GOT_REQUEST", AXI_BUS::transaction_to_string(trans));

	// give delay
	if (AXI_SUBORDINATE_LATENCY > 0)
	{
		wait (AXI_SUBORDINATE_LATENCY);
	}

	uint64_t amount_addr_inc = DATA_WIDTH / 8;
	for (int i = 0; i < trans.length; i ++)
	{
		uint64_t addr = trans.addr + amount_addr_inc * i;

		if (trans.is_write)
		{
			map_memory[addr] = trans.data[i];
		}
		else
		{
			auto iter = map_memory.find(addr);
			if (iter != map_memory.end())
			{
				trans.data[i] = map_memory[addr];
			}
			else
			{
				SC_REPORT_FATAL("Address out of range", AXI_BUS::transaction_to_string(trans).c_str());
			}
		}
	}

	is_accepted = response.nb_write(trans);
	if (is_accepted == false)
	{
		log(__FUNCTION__, "START_PENDING", AXI_BUS::transaction_to_string(trans));
		has_pending_response = true;
		trans_pending = trans;
	}
	else
	{
		log(__FUNCTION__, "SENT_RESPONSE", AXI_BUS::transaction_to_string(trans));
	}

}

void AXI_SUBORDINATE::read_memory_csv()
{
	map_memory.clear();

	std::ifstream f(filename_memory);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename_memory << std::endl;
		return;
	}

	int line_number = 0;
	std::string line;
	while (std::getline(f, line))
	{
		std::tuple<uint64_t, bus_data_t> row;
		std::istringstream iss(line);
		uint64_t address;
		bus_data_t data;
		std::string token1, token2;
		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		if (token1.empty() || token2.empty())
		{
			std::cerr << "Error: invalid format in " << filename_memory << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		address = address_from_hex_string(token1);
		data = bus_data_from_hex_string(token2);
		map_memory[address] = data;
		line_number ++;
	}
}

void AXI_SUBORDINATE::write_memory_csv(const char *filename)
{
	std::ofstream f(filename);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename << std::endl;
		return;
	}

	std::map<uint64_t, bus_data_t> map_ordered(map_memory.begin(), map_memory.end());
	for (auto row : map_ordered)
	{
		uint64_t address = std::get<0>(row);
		bus_data_t data = std::get<1>(row);
		f << "0x" << std::setfill('0') << std::setw(ADDR_WIDTH / 4) << std::hex << address;
		f << ","  << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(SC_HEX) << std::endl;
	}
}

void AXI_SUBORDINATE::log(std::string source, std::string action, std::string detail)
{
	std::string log_source = "SUBORDINATE:" + source;
	AXI_BUS::log(log_source, action, detail);
}
