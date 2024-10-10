.data
x: .word 0
y: .word 0
z: .space 16
a: .word 0
b: .word 0
c: .word 0
t15: .word 0
t13: .word 0
t11: .word 0
t9: .word 0
.text
.globl main
main:
# Generating MIPS code for array assignment
	la $t8, z
	li $t0, 3
	sw $t0, 0($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t1, 5
	sw $t1, 4($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t1, 7
	sw $t1, 8($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t1, 9
	sw $t1, 12($t8)
# Generating MIPS code for write operation
	li $a0, 25
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	li $a0, 90
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for array access
	la $t8, z
	lw $t1, 0($t8)
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t9 back to memory
	sw $t1, t9
# Generating MIPS code for array access
	la $t8, z
	lw $t1, 4($t8)
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t11 back to memory
	sw $t1, t11
# Generating MIPS code for array access
	la $t8, z
	lw $t1, 8($t8)
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t13 back to memory
	sw $t1, t13
# Generating MIPS code for array access
	la $t8, z
	lw $t1, 12($t8)
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t15 back to memory
	sw $t1, t15
	li $v0, 10
	syscall
