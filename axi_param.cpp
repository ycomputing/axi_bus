#include <iostream>
#include <sstream>
#include <iomanip>
#include <systemc>
#include "axi_param.h"

uint64_t address_from_hex_string(const std::string& s)
{
	std::stringstream ss;
	ss << std::hex << s;
	uint64_t address;
	ss >> address;
	return address;
}

std::string address_to_hex_string(uint64_t address)
{
	std::stringstream ss;
	ss << "0x" << std::setfill('0') << std::setw(ADDR_WIDTH / 4) << std::hex << address;
	return ss.str();
}

bus_data_t bus_data_from_hex_string(const std::string& s)
{
	std::stringstream ss;
	ss << std::hex << s;
	bus_data_t data;
	ss >> data;
	return data;
}

std::string bus_data_to_hex_string(const bus_data_t& data)
{
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(sc_dt::SC_HEX);
	return ss.str();
}

std::string bus_info_to_string(const axi_bus_info_t& info)
{
	std::string s;
	s = "id=" + std::to_string(info.id)
		+ ", addr=" + address_to_hex_string(info.addr)
		+ ", len=" + std::to_string(info.len)
		+ ", data=" + bus_data_to_hex_string(info.data);
	return s;
}

std::string transaction_to_string(const axi_trans_t& trans)
{
	std::string s;
	bool is_first = true;
	s = "addr=" + address_to_hex_string(trans.addr)
		+ ", progress=" + std::to_string(trans.progress)
		+ "/" + std::to_string(trans.length)
		+ ", wr=" + std::to_string(trans.is_write)
		+ ", data=";
	for (int i = 0; i < trans.length; i++)
	{
		if (is_first == false)
		{
			s += ",";
		}
		s += bus_data_to_hex_string(trans.data[i]);
		if (is_first)
		{
			is_first = false;
		}
	}
	return s;
}
