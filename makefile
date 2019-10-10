CC=gcc
CFLAGS = -O3 -Wall -fopenmp -march=native
LIBS = -lm -lgomp
# Directory to keep object files:
ODIR = obj
IDIR = include

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

.PHONY: all
all: cell_distances

# Rule to generate object files:
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -flto -c -o $@ $< $(CFLAGS) -I$(IDIR)

cell_distances: $(ODIR)/cell_distances.o 
	$(CC) -flto -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: test clean # Avoid conflict with a file of the same name
clean:
	rm -rvf $(ODIR)/*.o cell_distances cell_distances.tar.gz extracted/ pictures/

test:
	tar -czvf cell_distances.tar.gz *
	./check_submission.py cell_distances.tar.gz
