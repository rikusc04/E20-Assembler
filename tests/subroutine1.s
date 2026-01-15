# Example of simple subroutine. At the jal
# insruction, we call subroutine proc,
# then return from it, to the address
# after the subroutine invocation. Thus,
# the addi instructions will be executed
# in the following order:
#   X1, X2, X3

    addi $1, $0, 1     # X1: assign 1 to $1
    jal proc           # call subroutine proc
    addi $2, $0, 2     # X3: assign 2 to $2 
    halt

proc:
    addi $3, $0, 3     # X2: assign 3 to #3
    jr $7

#--
#--
#--MACHINE CODE
# ram[0] = 16'b0010000010000001;		// addi $1,$0,1
# ram[1] = 16'b0110000000000100;		// jal proc
# ram[2] = 16'b0010000100000010;		// addi $2,$0,2
# ram[3] = 16'b0100000000000011;		// halt 
# ram[4] = 16'b0010000110000011;		// proc: addi $3,$0,3
# ram[5] = 16'b0001110000001000;		// jr $7
#--
#--