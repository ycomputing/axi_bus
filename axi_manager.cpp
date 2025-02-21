#include <iostream>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>

using namespace sc_core;
using namespace sc_dt;

#include "axi_manager.h"

void AXI_MANAGER::thread()
{
	on_reset();
	wait();

	while(true)
	{
		on_clock();
		wait();
	}
}

void AXI_MANAGER::on_clock()
{
	fifo_manager();
}

void AXI_MANAGER::on_reset()
{

}

void AXI_MANAGER::log(std::string source, std::string action, std::string detail)
{
	std::string log_source = "MANAGER:" + source;
	AXI_BUS::log(log_source, action, detail);
}

void AXI_MANAGER::fifo_manager()
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";
	bool is_accepted;
	axi_trans_t trans;

	// check response first
	is_accepted = response.nb_read(trans);
	if (is_accepted)
	{
		if (trans.is_write == false)
		{
			uint64_t amount_addr_inc = DATA_WIDTH / 8;
			// update memory
			for (int i = 0; i < trans.length; i ++)
			{
				uint64_t addr = trans.addr + amount_addr_inc * i;
				map_memory[addr] = trans.data[i];
			}
		}
		log_detail = AXI_BUS::transaction_to_string(trans);
		log(__FUNCTION__, "GOT RESPONSE", log_detail);
	}

	// Are there requests to send?

	if (queue_access.empty())
	{
		// No job to do.
		log_action = "empty q";
		log(__FUNCTION__, log_action, log_detail);
		return;
	}

	// queue access tuple: (timestamp, rw, address, length, data)
	auto tuple = queue_access.front();
	uint64_t stamp_q = std::get<0>(tuple);
	trans = std::get<1>(tuple);

	// XXX Warning:
	// sc_time_stamp().value() depends on the time resolution of the simulation.
	// It is not guaranteed to be the same as the timestamp in the CSV file.
	// So we assume that the timestamp in the CSV file is the NS(nano seconds),
	// whereas the timestamp in the simulation is the PS(pico seconds).

	uint64_t stamp_now = sc_time_stamp().value() / 1000; // ps to ns convert.

	if (stamp_q > stamp_now)
	{
		// The time has not come yet
		log_action = CHANNEL_HOLD;
		log_detail = "scheduled=" + std::to_string(stamp_q)
				+ ",now=" + std::to_string(stamp_now);
		log(__FUNCTION__, log_action, log_detail);
		return;
	}

	is_accepted = request.nb_write(trans);
	if (is_accepted)
	{
		log_detail = AXI_BUS::transaction_to_string(trans);
		log(__FUNCTION__, "request", log_detail);
		queue_access.pop();
	}
}

void AXI_MANAGER::read_access_csv()
{
	while (!queue_access.empty())
	{
		queue_access.pop();
	}

	std::ifstream f(filename_access);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename_access << std::endl;
		return;
	}

	axi_trans_t trans_current;
	uint64_t stamp_current;
	bool is_expecting_new = true;
	int count_data = 0;
	std::tuple<uint64_t, axi_trans_t> row;

	int line_number = 1;
	std::string line;
	while (std::getline(f, line))
	{
		std::istringstream iss(line);
		uint64_t stamp;
		char rw;
		bool is_write;
		uint64_t address;
		uint8_t length;
		bus_data_t data;
		std::string token1, token2, token3, token4, token5;

		// line format
		// stamp, R/W, address, length, data
		//
		// stamp: integer, simulation time in nano second
		// R/W: character, 'R' or 'W', to indicate read or write action
		// address: hex string, 64 bit, address to read or write
		// length: integer, how many data to transfer at one transaction
		// data: hex string, 128 bit, data to transfer

		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		std::getline(iss, token3, ',');
		std::getline(iss, token4, ',');
		std::getline(iss, token5, ',');
		if (token1.empty() || token2.empty() || token3.empty() || token4.empty() || token5.empty())
		{
			std::cerr << "Error: invalid format in " << filename_access << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}

		stamp = std::stoull(token1);
		rw = token2[0];
		address = std::stoull(token3, nullptr, 16);
		length = std::stoul(token4);
		data = token5.c_str();

		if (rw == BUS_ACCESS_WRITE)
		{
			is_write = true;
		}
		else if (rw == BUS_ACCESS_READ)
		{
			is_write = false;
		}
		else
		{
			// This must not happen
			SC_REPORT_FATAL("AXI_MANAGER", "Invalid access type. It must be R or W");
		}

		if(is_expecting_new)
		{
			trans_current.addr = address;
			trans_current.length = length;
			trans_current.data[0] = data;
			trans_current.is_write = is_write;
			count_data = 1;
			stamp_current = stamp;
			is_expecting_new = false;
		}
		else	// expecting more data
		{
			trans_current.data[count_data] = data;
			count_data ++;
			if ((length != trans_current.length) | (stamp != stamp_current))
			{
				// This must not happen
				std::cerr << "Error: invalid access length in " << filename_access
					<< ", at line (" << line_number << "): " << line << std::endl;
				SC_REPORT_FATAL("AXI_MANAGER", "Invalid access length");
			}
		}

		if (length >= AXI_TRANSACTION_LENGTH_MAX)
		{
			std::cerr << "Error: too long access length " << std::to_string(length)
				<< " (max=" << AXI_TRANSACTION_LENGTH_MAX << ") in " << filename_access
				<< ", at line (" << line_number << "): " << line << std::endl;
			SC_REPORT_FATAL("AXI_MANAGER", "Too long access length");
		}

		if (count_data == length)
		{
			row = std::make_tuple(stamp_current, trans_current);
			queue_access.push(row);
			is_expecting_new = true;
		}
		line_number ++;
	}
}

void AXI_MANAGER::write_memory_csv(const char *filename)
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
