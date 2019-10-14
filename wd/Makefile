CC := gcc
CFLAGS := -std=c11 -O3 -Wall -pthread -flto -march=native -ffast-math -fopenmp 
LIBS := -lpthread -lgomp

OBJS := cell_distances.o 

.PHONY: all clean
all: cell_distances 

# Rule to generate object files:
cell_distances: $(OBJS) 
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LIBS)

$(OBJS) : helper.h

clean:
	-rm -f *.o cell_distances
