# We're testing that write-through works.
# A write should cache the value,
# even if it is already in the cache.

movi $1, 42
lw $2, 50($0) # this should always be a miss
sw $1, 50($0)


lw $2, 50($0) # this should always be a hit

halt
#--
#--
#--MACHINE CODE
# ram[0] = 16'b0010000010101010;		// movi $1,42
# ram[1] = 16'b1000000100110010;		// lw $2,50($0)
# ram[2] = 16'b1010000010110010;		// sw $1,50($0)
# ram[3] = 16'b1000000100110010;		// lw $2,50($0)
# ram[4] = 16'b0100000000000100;		// halt 
#--
#--