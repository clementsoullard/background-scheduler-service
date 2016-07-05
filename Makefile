prefix=/usr/local

all: scheduler
	
scheduler: scheduler.o adc.o
	$(CXX) $+ -o $@ -lwiringPi

install: scheduler
	install -m 755 scheduler $(prefix)/bin

uninstall:
	$(RM) $(prefix)/bin/scheduler

clean:
	$(RM) *.o scheduler