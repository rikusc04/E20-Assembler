# A simple loop, counting down from 63
    movi $1, 63         # Initialize counter
    movi $6, 1
    j beginning
done:
    halt                # we've finished
beginning:
    slti $5, $1, 10
    jeq $5, $6,done
    addi $1, $1, -3     # Decrement $1
    j beginning         # go to top of loop
#--
#--
#--MACHINE CODE
# ram[0] = 16'b0010000010111111;		// movi $1,63
# ram[1] = 16'b0010001100000001;		// movi $6,1
# ram[2] = 16'b0100000000000100;		// j beginning
# ram[3] = 16'b0100000000000011;		// done: halt 
# ram[4] = 16'b1110011010001010;		// beginning: slti $5,$1,10
# ram[5] = 16'b1101011101111101;		// jeq $5,$6,done
# ram[6] = 16'b0010010011111101;		// addi $1,$1,-3
# ram[7] = 16'b0100000000000100;		// j beginning
#--
#--