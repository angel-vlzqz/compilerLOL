.data
spill_area: .word 0
arr: .space 20
scalar3: .word 10
scalar2: .float 10.500000
scalar: .float 5.500000
x: .word 0
y: .word 0
z: .float 0.0
result: .float 0.0
i: .word 3
sum: .float 0.0
.text
main:
.globl main
# Prologue for function main
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
# Generating MIPS code for array assignment
	la $t8, arr
	li $t0, 7
	sw $t0, 12($t8)
# Assignment
	li $t0, 4
	move $t1, $t0
# Generating MIPS code for array assignment
	la $t8, arr
	sw $t2, 16($t8)
	move $a0, $t2
# Array load
	la $t8, arr
	lw $t3, 12($t8)
	move $a1, $t3
# Array load
	la $t8, arr
	lw $t3, 16($t8)
	move $a2, $t3
# Function call to addAndMultiply
	jal addAndMultiply
# Integer arithmetic -
	li $t3, 1
	sub $t4, $t1, $t3
# Array load
	la $t8, arr
	mul $t9, $t4, 4
	add $t9, $t8, $t9
	lw $t3, 0($t9)
# Array load
	la $t8, arr
	mul $t9, $t1, 4
	add $t9, $t8, $t9
	lw $t3, 0($t9)
# Generating MIPS code for write_float operation
	mov.s $f12, $f0
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write_float operation
	l.s $f12, sum
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write_float operation
	l.s $f12, scalar
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable scalar back to memory
	sw $t2, scalar
# Generating MIPS code for write_float operation
	l.s $f12, scalar2
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	lw $a0, scalar3
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Array load
	la $t8, arr
	lw $t1, 16($t8)
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Epilogue for function main
main_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	li $v0, 10
	syscall

addAndMultiply:
	l.s $f12, z
	move $a1, $t0
	move $a2, $t0
# Prologue for function addAndMultiply
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
# Epilogue for function addAndMultiply
addAndMultiply_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	jr $ra
