.data
.text
.globl main
main:
	lw $t0, 12
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t10
	lw $t0, 1
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t14
	lw $t0, 1
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t16
	lw $t0, t16
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t17
	lw $t0, 1
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t19
	lw $t0, t19
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t20
	lw $t0, t20
	lw $t1, 3
	add $t2, $t0, $t1
	sw $t2, t21
	lw $t0, 1
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t24
	lw $t0, t24
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t25
	lw $t0, t25
	lw $t1, 3
	add $t2, $t0, $t1
	sw $t2, t26
	lw $t0, t26
	lw $t1, 12
	add $t2, $t0, $t1
	sw $t2, t27
	li $t0, 1
	move $a0, $t0
	li $v0, 1
	syscall
	lw $t0, y
	move $a0, $t0
	li $v0, 1
	syscall
	li $v0, 10
	syscall
