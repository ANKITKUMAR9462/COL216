.data
stringadrr: .word 0

.text
.globl main
main:
	
	#allocating the memory in heap
	li $v0,9
	li $a0,100
	syscall
	sw $v0,stringadrr
        #given 100 byte memory in heap
	
	#taking input text from the user
	li $v0, 8
	lw $a0, stringadrr                     #input buffer
	la $a1, 100                            #max length of string
	syscall

	#printing out the input text
	li $v0, 4
	lw $a0, stringadrr
	syscall	

	#end of main
	li $v0, 10
	syscall
