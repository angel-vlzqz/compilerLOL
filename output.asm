.data
.text
.globl main
main:
	li $t0, 1
	sw $t0, t0
	li $t0, 12
	sw $t0, t1
	li $t0, 23
	sw $t0, t2
	li $t0, 10
	sw $t0, t3
	li $t0, 3
	sw $t0, t4
	li $t0, 1
	sw $t0, t5
	li $t0, 5
	sw $t0, t6
	li $v0, 10
	syscall
