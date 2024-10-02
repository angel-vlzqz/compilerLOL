.data
t13: .word 0
t11: .word 0
c: .word 0
t10: .word 0
.text
.globl main
main:
# Generating MIPS code for operation +
	li $t0, 70
	lw $t1, c
	add $t2, $t0, $t1
	sw $t2, t10
# Generating MIPS code for operation +
	lw $t0, t10
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t11
# Generating MIPS code for operation +
	lw $t0, t11
	li $t1, 5
	add $t2, $t0, $t1
	sw $t2, t13
# Generating MIPS code for write operation
	li $a0, 25
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	lw $a0, t13
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	li $v0, 10
	syscall
