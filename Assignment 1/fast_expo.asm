#equivalent c code used

#power(int x, int n)
#{
# if(n==0)
#  return 1;
#  int temp=power(x,n/2);
  
# if(n%2==0)
#  return temp*temp;
# else
# return x*temp*temp;

# }

.data

.text

main:

li $v0, 5
syscall	               #taking the number 
move $t1,$v0


li $v0, 5
syscall	            #taking the exponent
move $t2,$v0


move $a0,$t1             # this is x in above c code
move $a1,$t2           #this is n in above c code
li $v1,0                     # this is the final register where i will be returning the value of power function
j print                   # created a label for printing the  value given by power function



returning:                        #made this label for handligng the case that if(n==0) return 1
li $v1,1                                  #assigning the value 1 to returnning register v1
lw $ra,($sp)                                   #loading the return adress to go back
addiu $sp,$sp,16                           #deleting the stack used by this call
jr $ra 						#jump back to caller 

if1: 					#this is the fist if condition equivalent c code if(n%2)==0 return temp*temp
mul $v1,$v1,$v1
lw $ra,($sp)
addiu $sp,$sp,16
jr $ra

else:
lw $a0,4($sp)                   #this is the else condition equivalent c code return x*temp*temp
mul $v1,$v1,$v1
mul $v1,$v1,$a0
lw $ra,($sp)
addiu $sp,$sp,16
jr $ra


recurse:
lw $a1,8($sp) 
div  $a1,$a1,2                   # this is the recursive step equivalent c code is int temp=power(x,n/2)
j power


power:
subu $sp,$sp,16
sw $ra,($sp)                                  # this is the main power function that we implemented 
sw $a0,4($sp)
sw $a1,8($sp)                          #every call grows its stack by 16 bits to allocate the storage for x n temp and the most important return adress
sw $v1,12($sp)

li $t1,0
lw $a1,8($sp)
beq $a1,$t1 returning                  #if (n==0) return 1
jal recurse                              # temp=power(x,n/2)

lw $a1,8($sp)                        
rem $t6,$a1,2                              
beq $t6,$zero if1                         # checking  if(n%2==0)  if yes then jump to if1 else jump to else
j else


print:
jal power
li $v0,1                                #main  function for printing the exponented value
move $a0,$v1
syscall

#end final
li $v0, 10
syscall
