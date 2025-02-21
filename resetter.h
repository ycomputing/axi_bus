SC_MODULE(RESETTER)
{
	// output ports
	sc_out<bool>	ARESETn;

	SC_CTOR(RESETTER)
	{
		SC_THREAD(thread_execute);
	}

	void thread_execute()
	{
		ARESETn = true;
		wait(13, SC_NS);
		ARESETn = false;
		wait(20, SC_NS);
		ARESETn = true;
		wait(40, SC_NS);
		ARESETn = false;
		wait(10, SC_NS);
		ARESETn = true;
	}
};
