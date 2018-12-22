.SUFFIXES:
.SUFFIXES: .c .o .h

EXAMPLE = mine

CC	= gcc
CXX	= g++
DEFINES = -DINC=\"$(EXAMPLE).h\" -DOFILE=\"$(EXAMPLE).out\"
CFLAGS	= -g -O3 -march=native -fomit-frame-pointer -Wall -I.
CXXFLAGS = $(CFLAGS)

SRCS	= aha.c
OBJS	= $(SRCS:%.c=%.o)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.c.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<



all: aha

%: %.h aha.c
	$(CXX) -DINC=\"$<\" -DOFILE=\"$@.out\" $(CXXFLAGS) -o $@ aha.c

%seq: %.h ahaseq.c
	$(CXX) -DINC=\"$<\" -DOFILE=\"$@.out\" $(CXXFLAGS) -o $@ ahaseq.c
#.PHONY: aha
aha: $(OBJS)
	$(CXX) $(CXXFLAGS) $(DEFINES) -o aha $(OBJS)

clean:
	$(RM) -f $(OBJS) aha core *~ *.bak
