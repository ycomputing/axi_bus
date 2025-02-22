#!/usr/bin/env python3
import random
import os
import gen_random_access
import compare_memory

def test_one_random():
	gen_random_access.random_gen_random_access()
	os.system ("make run")
	is_different = compare_memory.compare_memory()
	return is_different

if __name__ == '__main__':
	random.seed(0)
	stop_now = False

	while(not stop_now):
		stop_now = test_one_random()
