CC := gcc
CFLAGS := -std=c11 -O3 -Wall -pthread -flto -march=native -ffast-math -fopenmp 
LIBS := -lpthread -lgomp

OBJS := cell_distances.o 

.PHONY: all test clean
all: cell_distances 

# Rule to generate object files:
cell_distances: $(OBJS) 
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LIBS)

$(OBJS) : helper.h

test:
	tar -czvf cell_distances.tar.gz cell_distances.c helper.h Makefile
	./check_submission.py cell_distances.tar.gz
clean:
	rm -rvf *.o cell_distances distances/ extracted/ cell_distances.tar.gz
