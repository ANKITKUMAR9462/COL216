
#Equivalent c code

#int main()
#{
 #   int n;
 #   cin>>n;
 #int v[n],c;
 #for(int i=0;i<n;i++)
 #{ 
 # cin>>c;
 # v[i]=c;
 # }
 #   int s=0,e=n-1,temp=0,x;
 #  cin>>x;
 #   while(s<=e){
 #       temp=(s+e)/2;
 #      if(v[temp]==x){
 #           cout<<"Yes at index "<<temp;
 #           return 0;
 #       }else if(v[temp]<x){
 #           s=temp+1;
 #       }else{
 #           e=temp-1;
 #       }
 # }
 #   cout<<"Not found";
 # return 0;
 #}

.data
	#array: .space 120
	arradrr: .word 0
  	text1: .asciiz "Yes at index "
	text2: .asciiz "Not found"

.text
main:
	
	#allocating the memory in heap
	li $v0,9
	li $a0,120
	syscall
	sw $v0,arradrr
       
         #given 120 byte memory in heap

	#read n and store in $s0
	li $v0, 5
	syscall	
	move $s0, $v0
	
	
	
	addi $t0, $zero, 0
	sll    $t1, $s0, 2                         #see here for next beq error
	whilea:
		bge $t0, $t1, exita
		li $v0, 5
		syscall
		lw $t6,arradrr
		add $t6,$t6,$t0
		sw $v0,($t6)
		addi $t0, $t0, 4
		j whilea  
	exita:	
	   	#array stored
	
	 
	#reading x and storing it in $s1

	li $v0, 5
	syscall
	
	move $s1, $v0

	# reading part done above
	# algorithm from here
	
	addi $s2, $zero, 0                 # initializing start index s, $s2
	addi $s3, $s0, -1                   # intializing end index e, $s3
			       #addi $t1, $zero, 0                 

	while:
		bgt $s2, $s3, exit
		add $t0, $s2, $s3           
		srl   $t0, $t0, 1                 # temp= (s+e)/2
		sll   $t1, $t0, 2                 # $t1=temp*4
		
		lw $t6,arradrr
		add $t6,$t6,$t1
		lw   $t2, ($t6)        # array[temp]
		beq $t2, $s1, branch1    #array[temp]=x
		blt   $t2, $s1, branch2    #array[temp]<x
		addi $s3, $t0, -1              #when array[temp]>x, e=temp-1
		j while

		branch2:
			addi $s2, $t0, 1
			j while

	exit:
		li $v0, 4
		la $a0, text2
		syscall
		j final_exit

	branch1:
		#printing "Yes at index "
		
		li $v0, 4
		la $a0, text1
		syscall	
	
		#printing the index temp
		
		li $v0, 1
		move $a0, $t0
		syscall

		j final_exit
	
	final_exit:
		#exit of main
		li $v0, 10
		syscall                           
