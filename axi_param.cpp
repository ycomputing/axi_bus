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

std::string bus_data_to_hex_string(bus_data_t data)
{
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(sc_dt::SC_HEX);
	return ss.str();
}
