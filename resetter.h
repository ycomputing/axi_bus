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
		ARESETn = false;
		wait(10, SC_NS);
		ARESETn = true;
	}
};
