#!/usr/bin/env python3
import random
import gen_random_access as g

if __name__ == '__main__':
	random.seed(0)  # for reproducibility
	g.gen_random_access(mode="any", length_max=4, is_length_variable=True, n=10)

#	g.gen_random_access()
#	g.gen_random_access(mode="any", n=1000)
#	g.gen_random_access(mode="any", length_max=1, is_length_variable=False, n=10)
#	g.gen_random_access(mode="any", length_max=100, is_length_variable=True, n=1000)
#	g.gen_random_access(mode="any", length_max=8, is_length_variable=True, stamp_step_min=10, n=1000)
#	g.gen_random_access(mode="any", length_max=4, is_length_variable=True, stamp_step_min=2, stamp_step_max=4, n=1000)
#	g.gen_random_access(mode="any", length_max=8, is_length_variable=True, stamp_step_min=1, stamp_step_max=2, n=1000)
#	g.gen_random_access(mode="any", length_max=1, is_length_variable=False, n=1000)
