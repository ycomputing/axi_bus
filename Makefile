SYSTEMC		:= /opt/systemc
INCLUDE_DIR	:= $(SYSTEMC)/include
LIB_DIR		:= $(SYSTEMC)/lib-linux64
CXX		:= g++
CFLAGS		:= -c -g -Wall -I$(INCLUDE_DIR)
CXXFLAGS	:= -std=c++17
LDFLAGS		:= -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR)
LIBS		:= -lsystemc -lm
EXE		:= project.exe

SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPEND	= $(OBJS:%.o=%.d)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt
	@test -x $@

-include $(DEPEND)

.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) -MMD -c $< -o $@

view:	$(EXE)
	./$(EXE)
	gtkwave trace.vcd

clean:
	rm -f $(OBJS) $(EXE) $(DEPEND) *.out trace.vcd

test:	$(EXE)
	python3 gen_random_access.py
	./$(EXE) > run.out
	python3 compare_memory.py

