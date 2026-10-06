FC = gfortran
FFLAGS = -std=legacy
MACHINE_SOURCE ?= ../lib/linux/machine.f
BUILD_DIR ?= build
MORTRAN_EXE ?= $(BUILD_DIR)/mortran3_inc.exe

.PHONY: all
all: $(MORTRAN_EXE)

$(MORTRAN_EXE): $(BUILD_DIR)/mortran3_inc.o $(BUILD_DIR)/machine.o
	mkdir -p "$(dir $@)"
	$(FC) $(FFLAGS) -o "$@" $^

$(BUILD_DIR)/mortran3_inc.o: mortran3.f
	mkdir -p "$(dir $@)"
	$(FC) $(FFLAGS) -ffree-form -c -o "$@" "$<"

$(BUILD_DIR)/machine.o: $(MACHINE_SOURCE)
	mkdir -p "$(dir $@)"
	$(FC) $(FFLAGS) -ffixed-form -ffixed-line-length-none -c -o "$@" "$<"