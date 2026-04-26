# Compilers
HOST_CXX = g++
RV_CXX = riscv64-unknown-elf-g++

# Flags
RV_FLAGS = -march=rv64gcv -O2
HOST_FLAGS = -O2 -lgtest -lgtest_main -pthread
QEMU = qemu-riscv64
QEMU_FLAGS = -cpu rv64,v=true,vlen=256

# Targets
all: canny_rv

# 1. Cross-compiles the pipeline for RISC-V
canny_rv:
	$(RV_CXX) $(RV_FLAGS) src/main.cpp -o build/canny_rv

# 2. Executes the RISC-V binary on QEMU
run: canny_rv
	$(QEMU) $(QEMU_FLAGS) ./build/canny_rv

# 3. Compiles and runs GoogleTest suite natively on the host
test:
	$(HOST_CXX) tests/test_main.cpp $(HOST_FLAGS) -o build/host_test
	./build/host_test

# 4. Removes all generated files
clean:
	rm -rf build/*
