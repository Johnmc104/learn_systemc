
CODETYPE=code_example
TESTNAME=ex02_counter

#---------------------------------------------------
LOC=  /opt/systemc-2.3.3
INCLS= -I $(LOC)/bin/include
LIBS=  -L $(LOC)/bin/lib-linux64
LD_LIBRARY_PATH=$(LOC)/bin/lib-linux64

CC= g++
CFLAGS= -g -Wall -Wextra -std=c++11 -Wno-write-strings
LSC= -lsystemc
EFILE=./simc
OBJS=sc_main.o


all:clean trans $(EFILE)
	export LD_LIBRARY_PATH
	$(EFILE)

trans:
	cp ../$(CODETYPE)/$(TESTNAME)/* .

$(EFILE):$(OBJS)
	#$(CC) $^ -o $@
	$(CC) -o $@ $^ $(LIBS) $(LSC)
	
%.o:%.cpp
	$(CC) $(CFLAGS) $(INCLS) -c $^

clean:
	@rm -rf *.o *.vcd *.cpp $(EFILE)

