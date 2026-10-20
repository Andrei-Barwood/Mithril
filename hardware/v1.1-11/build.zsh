# Step 1: Compile the C code to object file
gcc -c -fPIC -I/usr/local/include/iverilog div_with_rem_vpi.c -o div_with_rem_vpi.o

# Step 2: Create shared library
gcc -shared -o div_with_rem.vpi div_with_rem_vpi.o

# Step 3: Compile Verilog with VPI module
iverilog -o test_div_rem.vvp test_div_rem.v

# Step 4: Run simulation with VPI module loaded
vvp -M. -mdiv_with_rem test_div_rem.vvp
