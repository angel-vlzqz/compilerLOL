.data
x: .word 0
y: .word 0
z: .space 16
angel: .word 0
adon: .word 0
a: .word 0
b: .word 0
c: .word 0
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
# Generating MIPS code for write operation
	li $a0, 1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	li $a0, 0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	li $v0, 10
	syscall
