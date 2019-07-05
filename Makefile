CC=g++
CFLAGS=-pthread -std=gnu++11 -Wall

build: battery_stats_avg.cpp
	$(CC) battery_stats_avg.cpp -o battery_stats_avg $(CFLAGS)

clean:
	rm battery_stats_avg
