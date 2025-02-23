#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

#define SIMULATION_TIME	100000

#include "axi_bus.h"
#include "axi_manager.h"
#include "axi_subordinate.h"
#include "resetter.h"

int sc_main(int, char*[])
{
	sc_clock ACLK("ACLK", 1, SC_NS);
	sc_signal<bool> ARESETn;
	
	sc_fifo<axi_trans_t> request_M;
	sc_fifo<axi_trans_t> request_S;
	sc_fifo<axi_trans_t> response_M;
	sc_fifo<axi_trans_t> response_S;

	AXI_BUS bus("bus");
	AXI_MANAGER m("M1");
	AXI_SUBORDINATE s("S1");
	RESETTER r("r");
	
	r.ARESETn(ARESETn);

	bus.ACLK(ACLK);
	bus.ARESETn(ARESETn);
	bus.request_M(request_M);
	bus.response_M(response_M);
	bus.response_S(response_S);
	bus.request_S(request_S);

	m.request(request_M);
	m.response(response_M);

	s.request(request_S);
	s.response(response_S);

	sc_trace_file* f = sc_create_vcd_trace_file("trace");
	sc_trace(f, ARESETn, "ARESETn");
	sc_trace(f, ACLK, "ACLK");
	sc_trace(f, bus.AWVALID, "AWVALID");
	sc_trace(f, bus.AWREADY, "AWREADY");
	sc_trace(f, bus.AWID, "AWID");
	sc_trace(f, bus.AWADDR, "AWADDR");
	sc_trace(f, bus.AWLEN, "AWLEN");
	sc_trace(f, bus.WVALID, "WVALID");
	sc_trace(f, bus.WREADY, "WREADY");
	sc_trace(f, bus.WID, "WID");
	sc_trace(f, bus.WDATA, "WDATA");
	sc_trace(f, bus.WLAST, "WLAST");
	sc_trace(f, bus.BVALID, "BVALID");
	sc_trace(f, bus.BREADY, "BREADY");
	sc_trace(f, bus.BID, "BID");
	sc_trace(f, bus.ARVALID, "ARVALID");
	sc_trace(f, bus.ARREADY, "ARREADY");
	sc_trace(f, bus.ARID, "ARID");
	sc_trace(f, bus.ARADDR, "ARADDR");
	sc_trace(f, bus.ARLEN, "ARLEN");
	sc_trace(f, bus.RVALID, "RVALID");
	sc_trace(f, bus.RREADY, "RREADY");
	sc_trace(f, bus.RID, "RID");
	sc_trace(f, bus.RDATA, "RDATA");
	sc_trace(f, bus.RLAST, "RLAST");

	m.read_access_csv();
	s.read_memory_csv();
	
	sc_start(SIMULATION_TIME, SC_NS);

	m.write_memory_csv();
	s.write_memory_csv();

	sc_close_vcd_trace_file(f);
	return (0);
}
