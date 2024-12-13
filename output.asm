.data
	# Declaring array arr with size 20
arr: .space 20
	# Declaring integer variable scalar3 with initial value 10
scalar3: .word 10
	# Declaring float variable scalar2 with initial value 10.500000
scalar2: .float 10.500000
	# Declaring float variable scalar with initial value 5.500000
scalar: .float 5.500000
	# Declaring integer variable x with initial value 0
x: .word 0
	# Declaring integer variable y with initial value 0
y: .word 0
	# Declaring float variable z with initial value 0.0
z: .float 0.0
	# Declaring float variable done with initial value 0.0
done: .float 0.0
	# Declaring integer variable i with initial value 3
i: .word 3
	# Declaring float variable sum with initial value 0.0
sum: .float 0.0
	# Declaring float variable fcomp with initial value 0.0
fcomp: .float 0.0
	# Declaring integer variable comp with initial value 0
comp: .word 0
.text
addAndMultiply:	# Label for function addAndMultiply
	# Loading integer variable z into register $t0
	la $t1, z
	lw $t0, 0($t1)
	# Moving integer argument z into $a0
	move $a0, $t0
	# Loading integer variable y into register $t1
	la $t2, y
	lw $t1, 0($t2)
	# Moving integer argument y into $a1
	move $a1, $t1
	# Loading integer variable x into register $t2
	la $t3, x
	lw $t2, 0($t3)
	# Moving integer argument x into $a2
	move $a2, $t2
	# Prologue for function addAndMultiply
	addiu $sp, $sp, -8	# Allocate stack space
	sw $fp, 4($sp)	# Save frame pointer
	sw $ra, 0($sp)	# Save return address
	move $fp, $sp	# Set frame pointer
	# TAC: t2 = x + y
	# Loading integer variable x into register $t0
	la $t1, x
	lw $t0, 0($t1)
	# Loading integer variable y into register $t1
	la $t2, y
	lw $t1, 0($t2)
	add $t2, $t0, $t1	# Performing integer addition
	# TAC: t3 = t2 * z
	# Converting integer variable t2 in $t2 to float register $f2
	mtc1 $t2, $f2
	cvt.s.w $f2, $f2
	# Loading float variable z into register $f4
	la $t0, z
	lwc1 $f4, 0($t0)
	mul.s $f6, $f2, $f4	# Performing floating-point multiplication
	# Storing float variable t3 from register $f6 to memory
	la $t0, t3
	swc1 $f6, 0($t0)
	# Epilogue for function addAndMultiply
addAndMultiply_epilogue:	# Epilogue label
	move $sp, $fp	# Restore stack pointer
	lw $fp, 4($sp)	# Restore frame pointer
	lw $ra, 0($sp)	# Restore return address
	addiu $sp, $sp, 8	# Deallocate stack space
	jr $ra	# Return to caller
main:	# Label for function main
	.globl main	# Declare main as global
	# Prologue for function main
	addiu $sp, $sp, -8	# Allocate stack space
	sw $fp, 4($sp)	# Save frame pointer
	sw $ra, 0($sp)	# Save return address
	move $fp, $sp	# Set frame pointer
	# TAC: arr[3] = 7
	la $t8, arr
	# Loading integer constant 7 into register $t0
	li $t0, 7
	# Loading integer constant 3 into register $t1
	li $t1, 3
	sll $t9, $t1, 2	# Calculating byte offset for index
	add $t9, $t8, $t9	# Calculating address of arr[3]
	sw $t0, 0($t9)	# Storing value into arr[3]
	# Loading integer constant 10 into register $t0
	li $t0, 10
	# TAC: comp = 10
	move $t0, $t0	# Moving integer value to comp
	# Loading integer variable 10.333332 into register $t1
	la $t2, 10.333332
	lw $t1, 0($t2)
	# TAC: fcomp = 10.333332
	move $t1, $t1	# Moving integer value to fcomp
	# Loading integer constant 4 into register $t1
	li $t1, 4
	# TAC: i = 4
	move $t1, $t1	# Moving integer value to i
	# TAC: arr[4] = 7
	la $t8, arr
	# Loading integer constant 7 into register $t2
	li $t2, 7
	# Loading integer constant 4 into register $t3
	li $t3, 4
	sll $t9, $t3, 2	# Calculating byte offset for index
	add $t9, $t8, $t9	# Calculating address of arr[4]
	sw $t2, 0($t9)	# Storing value into arr[4]
	# Loading float variable scalar into register $f2
	la $t2, scalar
	lwc1 $f2, 0($t2)
	# Moving float argument scalar into $f12
	mov.s $f12, $f2
	# Loading integer constant 7 into register $t2
	li $t2, 7
	# Moving integer argument 7 into $a0
	move $a0, $t2
	# Loading integer constant 7 into register $t2
	li $t2, 7
	# Moving integer argument 7 into $a1
	move $a1, $t2
	# Calling function addAndMultiply
	jal addAndMultiply
	# TAC: t47 = v0 / scalar
	div.s $f4, $f0, $f2	# Performing floating-point division
	# Storing float variable t47 from register $f4 to memory
	la $t2, t47
	swc1 $f4, 0($t2)
	# TAC: t48 = i - 1
	# Loading integer constant 1 into register $t2
	li $t2, 1
	sub $t3, $t1, $t2	# Performing integer subtraction
	# TAC: t49 = arr[t48]
	la $t8, arr
	sll $t9, $t3, 2	# Calculating byte offset for index
	add $t9, $t8, $t9	# Calculating address of arr[t48]
	lw $t9, 0($t9)	# Loading value from arr[t48] into $t9
	# TAC: t50 = t49 * scalar
	# Converting integer variable t49 in $t9 to float register $f4
	mtc1 $t9, $f4
	cvt.s.w $f4, $f4
	mul.s $f6, $f4, $f2	# Performing floating-point multiplication
	# Storing float variable t50 from register $f6 to memory
	la $t3, t50
	swc1 $f6, 0($t3)
	# TAC: t51 = t47 + t50
