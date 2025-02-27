#include <iostream>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <mutex>

using namespace sc_core;
using namespace sc_dt;

#include "axi_bus.h"

// when you want to enable debug messages
// (comment out or) uncomment the following line.
#define DEBUG_AXI_BUS

// When you (don't) want to see channel signal activity
// (comment out or) uncomment the following line.
#define DEBUG_AXI_BUS_CHANNEL

void AXI_BUS::thread_clock()
{
	while(true)
	{
		if (ARESETn == 0)
		{
			on_reset();
		}
		else if (ACLK.posedge())
		{
			on_clock();
		}
		wait();
	}
}

void AXI_BUS::on_clock()
{
	channel_transaction();
}

void AXI_BUS::thread_request_M()
{
	while(true)
	{
		transaction_request_M();
	}
}

void AXI_BUS::thread_response_M()
{
	while(true)
	{
		transaction_response_M();
		wait();
	}
}

void AXI_BUS::thread_request_S()
{
	while(true)
	{
		transaction_request_S();
		wait();
	}
}

void AXI_BUS::thread_response_S()
{
	while(true)
	{
		transaction_response_S();
	}
}

void AXI_BUS::on_reset()
{
	AWVALID.write(0);
	AWREADY.write(0);
	AWID.write(0);
	AWADDR.write(0);
	AWLEN.write(0);

	WVALID.write(0);
	WREADY.write(0);
	WID.write(0);
	WDATA.write(0);
	WLAST.write(0);

	BVALID.write(0);
	BREADY.write(0);
	BID.write(0);

	ARVALID.write(0);
	ARREADY.write(0);
	ARID.write(0);
	ARADDR.write(0);
	ARLEN.write(0);

	RVALID.write(0);
	RREADY.write(0);
	RID.write(0);
	RDATA.write(0);
	RLAST.write(0);
}

bool AXI_BUS::is_address(int channel)
{
	switch (channel)
	{
		case CHANNEL_AW:	return(true);
		case CHANNEL_AR:	return(true);
		case CHANNEL_W:		return(false);
		case CHANNEL_B:		return(false);
		case CHANNEL_R:		return(false);
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
}

bool AXI_BUS::is_ready(int channel)
{
	switch (channel)
	{
		case CHANNEL_AW:	return(AWREADY);
		case CHANNEL_W:		return(WREADY);
		case CHANNEL_B:		return(BREADY);
		case CHANNEL_AR:	return(ARREADY);
		case CHANNEL_R:		return(RREADY);
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
}

bool AXI_BUS::is_valid(int channel)
{
	switch (channel)
	{
		case CHANNEL_AW:	return(AWVALID);
		case CHANNEL_W:		return(WVALID);
		case CHANNEL_B:		return(BVALID);
		case CHANNEL_AR:	return(ARVALID);
		case CHANNEL_R:		return(RVALID);
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
}

void AXI_BUS::set_valid(int channel, bool value)
{
	switch (channel)
	{
		case CHANNEL_AW:	AWVALID = value; break;
		case CHANNEL_W:		WVALID = value; break;
		case CHANNEL_B:		BVALID = value; break;
		case CHANNEL_AR:	ARVALID = value; break;
		case CHANNEL_R:		RVALID = value; break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

void AXI_BUS::set_ready(int channel, bool value)
{
	switch (channel)
	{
		case CHANNEL_AW:	AWREADY = value; break;
		case CHANNEL_W:		WREADY = value; break;
		case CHANNEL_B:		BREADY = value; break;
		case CHANNEL_AR:	ARREADY = value; break;
		case CHANNEL_R:		RREADY = value; break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

void AXI_BUS::send_info(int channel, axi_bus_info_t& info)
{
	switch(channel)
	{
		case CHANNEL_AW:	AWID = info.id;
							AWADDR = info.addr;
							AWLEN = info.len;
							break;
		case CHANNEL_W:		WID = info.id;
							WDATA = info.data;
							WLAST = info.is_last;
							break;
		case CHANNEL_B:		BID = info.id;
							break;
		case CHANNEL_AR:	ARID = info.id;
							ARADDR = info.addr;
							ARLEN = info.len;
							break;
		case CHANNEL_R:		RID = info.id;
							RDATA = info.data;
							RLAST = info.is_last;
							break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return;
	}
}

axi_bus_info_t AXI_BUS::create_null_info()
{
	axi_bus_info_t info;

	info.id = 0;
	info.addr = 0;
	info.len = 0;
	info.data = 0;
	info.is_last = false;
	return info;
}

axi_bus_info_t AXI_BUS::recv_info(int channel)
{
	axi_bus_info_t info = create_null_info();

	switch(channel)
	{
		case CHANNEL_AW:	info.id = AWID;
							info.addr = AWADDR;
							info.len = AWLEN;
							break;
		case CHANNEL_W:		info.id = WID;
							info.data = WDATA;
							info.is_last = WLAST;
							break;
		case CHANNEL_B:		info.id = BID;
							break;
		case CHANNEL_AR:	info.id = ARID;
							info.addr = ARADDR;
							info.len = ARLEN;
							break;
		case CHANNEL_R:		info.id = RID;
							info.data = RDATA;
							info.is_last = RLAST;
							break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
	}
	return (info);
}

void AXI_BUS::channel_receiver(int channel, std::queue<axi_bus_info_t>& q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";
	int size_q = 0;

	mutex_q.lock();
	size_q = q.size();
	
	if (is_ready(channel) == 0)
	{
		if (size_q >= AXI_BUS_Q_SIZE_RECV)
		{
			log_action = CHANNEL_NOT_READY;
		}
		else if (is_recv_full(channel))
		{
			log_action = CHANNEL_NOT_READY;
			set_ready(channel, false);
		}
		else
		{
			set_ready(channel, true);
			log_action = CHANNEL_READY;
		}
	}
	else if (is_valid(channel))	// ready and valid
	{
		axi_bus_info_t info = recv_info(channel);
		q.push(info);
		size_q = q.size();
		if (size_q >= AXI_BUS_Q_SIZE_RECV)
		{
			// Cannot receive more.
			log_action = CHANNEL_RECV_FULL;
			set_ready(channel, false);
		}
		else if (is_recv_full(channel))
		{
			// Cannot receive more.
			log_action = CHANNEL_RECV_FULL;
			set_ready(channel, false);
		}
		else
		{
			log_action = CHANNEL_RECV_CONTINUE;
		}
		log_detail = bus_info_to_string(info);
		event_something_to_send.notify(SC_ZERO_TIME);
	}
	else	// ready but not valid
	{
		if (is_recv_full(channel))
		{
			set_ready(channel, false);
			log_action = CHANNEL_NOT_READY;
		}
		else
		{
			log_action = CHANNEL_WAIT_V;
		}
	}

	log_detail = "recvQ=" + std::to_string(size_q);
	log(channel, log_action, log_detail);

	// keep asking to send
	event_something_to_send.notify(SC_ZERO_TIME);
	mutex_q.unlock();
}

void AXI_BUS::channel_sender(int channel, std::queue<axi_bus_info_t>& q)
{
	std::string log_action = CHANNEL_UNKNOWN;
	std::string log_detail = "";
	axi_bus_info_t info;

	mutex_q.lock();

	if (!q.empty())
	{
		if (is_ready(channel) == 0 && is_valid(channel) == 1)
		{
			// The receiver did not take current data yet.
			// We have to wait until the receiver is ready
			log_action = CHANNEL_WAIT_R;
		}
		else
		{
			info = q.front();

			if (is_valid(channel))
			{
				log_action = CHANNEL_SENDC;
			}
			else
			{
				log_action = CHANNEL_SEND;
			}

			send_info(channel, info);
			set_valid(channel, true);
			q.pop();

			log_action = CHANNEL_SEND;
			log_detail += ", " + bus_info_to_string(info);
		}
	}
	else	// Q is empty
	{
		if (is_ready(channel))
		{
			if (is_valid(channel))
			{
				// was not idle before, now idle.

				set_valid(channel, false);

				// AXI spec recommends to set zero when not in use
				info = create_null_info();
				send_info(channel, info);
				log_action = CHANNEL_IDLE;
			}
			else
			{
				// was idle before, now idle again
				log_action = CHANNEL_IDLE;
			}

		}
		else	// Q is empty and not ready
		{
			if (is_valid(channel))
			{
				log_action = CHANNEL_WAIT_R;
			}
			else
			{
				log_action = CHANNEL_IDLE;
			}
		}
	}

	log(channel, log_action, log_detail);
	mutex_q.unlock();
}

void AXI_BUS::log(int channel, std::string action, std::string detail)
{
#ifdef DEBUG_AXI_BUS_CHANNEL
	log(get_channel_name(channel), action, detail);
#endif
	return;
}

void AXI_BUS::log(std::string source, std::string action, std::string detail)
{

#ifdef DEBUG_AXI_BUS
	std::string out;
	out = sc_time_stamp().to_string() + ":" + source
		+ ":" + action + ":" + detail;
	std::cout << out << std::endl;
#endif

	return;
}

std::string AXI_BUS::get_channel_name(int channel)
{
	std::string channel_name;

	switch (channel)
	{
		case CHANNEL_AW:	channel_name = "AW";	break;
		case CHANNEL_W:		channel_name = "W";		break;
		case CHANNEL_B:		channel_name = "B";		break;
		case CHANNEL_AR:	channel_name = "AR";	break;
		case CHANNEL_R:		channel_name = "R";		break;
		default:			channel_name = "XXX";	break;
	}
	return channel_name;
}

void AXI_BUS::channel_transaction()
{
	channel_receiver(CHANNEL_AW, q_recv_AW);
	channel_receiver(CHANNEL_W, q_recv_W);
	channel_receiver(CHANNEL_AR, q_recv_AR);

	channel_sender(CHANNEL_AW, q_send_AW);
	channel_sender(CHANNEL_W, q_send_W);
	channel_sender(CHANNEL_AR, q_send_AR);

	//wait_enough_delta_cycles();

	channel_receiver(CHANNEL_B, q_recv_B);
	channel_receiver(CHANNEL_R, q_recv_R);

	channel_sender(CHANNEL_B, q_send_B);
	channel_sender(CHANNEL_R, q_send_R);
}

void AXI_BUS::transaction_request_M()
{
	axi_trans_t trans;

	// blocking read
	trans = request_M.read();
	log(__FUNCTION__, "GOT_REQUEST", transaction_to_string(trans));

	uint32_t id = generate_transaction_id();
	axi_bus_info_t info = create_null_info();
	info.id = id;
	info.addr = trans.addr;
	info.len = trans.length - 1;

	if (trans.is_write)
	{
		mutex_q.lock();
		q_send_AW.push(info);

		info.is_last = false;
		for (int i = 0; i < trans.length; i++)
		{
			if (i == info.len)
			{
				info.is_last = true;
			}
			info.data = trans.data[i];
			q_send_W.push(info);
		}
		mutex_q.unlock();
	}
	else
	{
		bool is_created = outstanding_R.create(info);
		while (is_created == false)
		{
			log(__FUNCTION__, "AR_WAIT_OUTSTANDING", transaction_to_string(trans));
			wait(event_outstanding_R_has_room.default_event());
			is_created = outstanding_R.create(info);
		}

		log(__FUNCTION__, "AR_CREATED_OUTSTANDING", transaction_to_string(trans));
		
		if (is_created)
		{
			mutex_q.lock();
			q_send_AR.push(info);
			mutex_q.unlock();
		}
	}

}

void AXI_BUS::transaction_response_S()
{
	axi_trans_t trans;
	axi_bus_info_t info;

	// blocking read
	trans = response_S.read();
	log(__FUNCTION__, "GOT_RESPONSE", transaction_to_string(trans));

	mutex_q.lock();

	uint32_t id;
	axi_trans_t trans_outstanding;
	bool found = false;
	std::unordered_map<uint32_t, axi_trans_t>::iterator iter;

	if (trans.is_write)
	{
		iter = outstanding_W.find_by_addr(trans.addr);
		if (iter != outstanding_R.map.end())
		{
			info = create_null_info();
			info.id = id;
			info.addr = trans.addr;
			info.len = trans.length - 1;
			q_send_B.push(info);
			outstanding_R.dump();
		}
	}
	else
	{
		// read
		iter = outstanding_W.find_by_addr(trans.addr);
		if (iter != outstanding_R.map.end())
		{
			info = create_null_info();
			info.id = id;
			info.addr = trans.addr;
			info.len = trans.length - 1;
			info.is_last = false;
			for (int i = 0; i < trans.length; i++)
			{
				if (i == info.len)
				{
					info.is_last = true;
				}
				info.data = trans.data[i];
				q_send_R.push(info);
			}
		}
	}

	mutex_q.unlock();
}

void AXI_BUS::transaction_response_M()
{
	std::string log_detail;
	axi_bus_info_t info;
	mutex_q.lock();
	// write transaction
	if (q_recv_B.empty())
	{
		mutex_q.unlock();
	}
	else
	{
		info = q_recv_B.front();
		q_recv_B.pop();
		log_detail = transaction_send_info(response_M, info);
		log(__FUNCTION__, "SENT RESPONSE", log_detail);
		outstanding_W.remove(info.id);
		mutex_q.unlock();
	}

	// read transaction
	mutex_q.lock();
	if (q_recv_R.empty())
	{
		mutex_q.unlock();
	}
	else
	{
		info = q_recv_R.front();
		bool is_completed = outstanding_R.update(info);
		if (is_completed)
		{
			log_detail = transaction_send_info(response_M, info);
			log(__FUNCTION__, "SENT RESPONSE", log_detail);
			outstanding_R.remove(info.id);
		}
		q_recv_R.pop();
		mutex_q.unlock();
	}
}

void AXI_BUS::transaction_request_S()
{
	mutex_q.lock();

	if (!q_recv_AW.empty())
	{
		log(__FUNCTION__, "AW_TRY_CREATE_OUTSTANDING", bus_info_to_string(q_recv_AW.front()));
		bool is_created = outstanding_W.create(q_recv_AW.front());
		while (is_created == false)
		{
			log(__FUNCTION__, "AW_WAIT_OUTSTANDING", bus_info_to_string(q_recv_AW.front()));
			mutex_q.unlock();
			wait(event_outstanding_W_has_room.default_event());
			mutex_q.lock();
			is_created = outstanding_W.create(q_recv_AW.front());
		};
		log(__FUNCTION__, "AW_CREATED_OUTSTANDING", bus_info_to_string(q_recv_AW.front()));
		q_recv_AW.pop();
	}

	mutex_q.unlock();
	mutex_q.lock();

	if (!q_recv_W.empty())
	{
		axi_bus_info_t info = q_recv_W.front();
		if (outstanding_W.is_id_present(info.id))
		{
			bool is_completed = outstanding_W.update(q_recv_W.front());
			if (is_completed)
			{
				axi_bus_info_t info = q_recv_W.front();
				q_recv_W.pop();
				transaction_send_info(request_S, info);
			}
		}
		else
		{
			// outstanding is not present. Not error but warning
			log(__FUNCTION__, "W is waiting AW", bus_info_to_string(info));
		}
	}

	mutex_q.unlock();
	mutex_q.lock();

	if (!q_recv_AR.empty())
	{
		axi_bus_info_t info = q_recv_AR.front();
		q_recv_AR.pop();
		mutex_q.unlock();
		transaction_send_info(request_S, info);
	}

	mutex_q.unlock();
}

std::string AXI_BUS::transaction_send_info(sc_fifo_out<axi_trans_t>& fifo_out, axi_bus_info_t& info)
{
	std::string log_detail;
	axi_trans_t trans;

	// This function does not lock the queue.
	// You must lock the queue before calling this function if needed.

	std::unordered_map<uint32_t, axi_trans_t>::iterator iter;
	iter = outstanding_R.find_by_id (info.id);
	if (iter != outstanding_R.map.end())
	{
		trans = iter->second;
		fifo_out.write(trans);
		log_detail = transaction_to_string(trans);
		return log_detail;
	}

	iter = outstanding_W.find_by_id (info.id);
	if (iter != outstanding_W.map.end())
	{
		trans = iter->second;
		fifo_out.write(trans);
		log_detail = transaction_to_string(trans);
		return log_detail;
	}

	// Nothing in outstanding for that id
	log_detail += ", " + bus_info_to_string(info);
	log(__FUNCTION__, "NO ID in outstanding", log_detail);
	outstanding_R.dump();
	outstanding_W.dump();
	SC_REPORT_FATAL("NOID", "q_recv_X");
	return log_detail;
}

bool AXI_BUS::is_recv_full(int channel)
{
	bool is_full = false;
	int size = 0;
	int vacancy = 0;
	switch (channel)
	{
		case CHANNEL_AW:	vacancy = outstanding_W.size_vacant();
							size = q_recv_AW.size();
							if (vacancy <= size)
							{
								is_full = true;
							}
							break;
		case CHANNEL_AR:	vacancy = outstanding_R.size_vacant();
							size = q_recv_AR.size();
							if (vacancy <= size)
							{
								is_full = true;
							}
							break;
		case CHANNEL_W:
		case CHANNEL_B:
		case CHANNEL_R:		is_full = false;
							break;
		default:
			SC_REPORT_FATAL("Unknown channel", std::to_string(channel).c_str());
			return false;
	}
	return is_full;
}

uint32_t AXI_BUS::generate_transaction_id()
{
	static uint32_t id = 0;
	id ++;
	if (id == 0)
	{
		id ++;
	}
	return id;
}

void AXI_BUS::wait_enough_delta_cycles()
{
	const int ENOUGH_DELTA_CYCLES = 10;
	for (int i = 0; i < ENOUGH_DELTA_CYCLES; i++)
	{
		wait(SC_ZERO_TIME);
	}
}
