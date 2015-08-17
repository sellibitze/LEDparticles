CXXFLAGS = -Wall -MMD -O

# Uncomment this if you want the program to output a PPM image stream
# with 500 frames (as rows). Might be useful for debugging.
# CXXFLAGS += -DWRITE_PPM

WAVES_BIN = waves
WAVES_OBJ = common.o waves.o

all: $(WAVES_BIN)

-include *.d

$(WAVES_BIN): $(WAVES_OBJ)
	$(CXX) -o $@ $(LDFLAGS) $(WAVES_OBJ)

clean:
	$(RM) $(WAVES_BIN) $(WAVES_OBJ) *.d
