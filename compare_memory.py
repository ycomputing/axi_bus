#!/usr/bin/env python3

import csv

DATA_BUS_WIDTH = 128
ADDR_INCREMENT_PER_TRANSFER = DATA_BUS_WIDTH / 8

def read_file_memory(filename):
	dict = {}
	f = open(filename)
	reader = csv.reader(f)
	for row in reader:
		addr = row[0]
		data = row[1]
		dict[addr] = data
	f.close()
	return dict

def read_file_access(filename):
	dict = {}
	f = open(filename)
	reader = csv.reader(f)
	for row in reader:
		stamp = row[0]
		rw = row[1]
		addr = row[2]
		length = row[3]
		data = row[4]
		if addr in dict:
			print("addr: %s already exists" % addr)
		dict[addr] = [stamp, rw, length, data]
	f.close()
	return dict

def compare_memory(filename_access="m_access.csv",
			filename_memory_m="m_memory_after.csv",
			filename_memory_s="s_memory_after.csv"):
	dict_access = read_file_access(filename_access)
	dict_m = read_file_memory(filename_memory_m)
	dict_s = read_file_memory(filename_memory_s)
	key_to_remove = []

	count_read = 0
	count_write = 0
	count_read_pass = 0
	count_write_pass = 0

	for addr in dict_access.keys():
		(stamp, rw, length, data) = dict_access[addr]
		if rw == "R":
			count_read += 1
			if addr not in dict_s:
				print("READ impossible: address %s" %(addr))
			elif addr not in dict_m:
				print("READ fail: address %s" %(addr))
			elif dict_m[addr] != dict_s[addr]:
				print("READ different value: address %s, m: %s, s: %s" % (addr, dict_m[addr], dict_s[addr]))
			else:
				count_read_pass += 1
		elif rw == "W":
			count_write += 1
			if addr not in dict_s:
				print("WRITE fail: address %s" %(addr))
			elif data != dict_s[addr]:
				print("WRITE different value: address %s, access data: %s, s: %s" % (addr, data, dict_s[addr]))
			else:
				count_write_pass += 1
		else:
			print("UNKNOWN action: address %s" %(addr))

	print("passed %s/%s read, %s/%s write actions" % (count_read_pass, count_read, count_write_pass, count_write))

	if (count_read_pass == count_read) and (count_write_pass == count_write):
		return 0
	else:
		return 1

if __name__ == "__main__":
	compare_memory()
