.data
.text
.globl main
main:
	li $t0, 1
	sw $t0, t0
	li $t0, 1
	sw $t0, x
	li $t0, 12
	sw $t0, t1
	li $t0, 12
	sw $t0, a
	li $t0, 23
	sw $t0, t2
	li $t0, 10
	sw $t0, t3
	li $t0, 23
	sw $t0, t4
	li $t0, 10
	sw $t0, t5
	lw $t0, t4
	lw $t1, t5
	add $t2, $t0, $t1
	sw $t2, t6
	lw $t1, (null)
	move $t0, $t1
	sw $t0, b
	li $t0, 3
	sw $t0, t7
	li $t0, 3
	sw $t0, c
	lw $t0, a
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t8
	li $t0, 1
	sw $t0, t9
	lw $t0, a
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t10
	li $t0, 1
	sw $t0, t11
	lw $t0, a
	lw $t1, t11
	add $t2, $t0, $t1
	sw $t2, t12
	lw $t1, (null)
	move $t0, $t1
	sw $t0, x
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t13
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t14
	lw $t0, x
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t15
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t16
	lw $t0, x
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t17
	lw $t0, x
	lw $t1, c
	add $t2, $t0, $t1
	sw $t2, t18
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t19
	lw $t0, x
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t0, x
	lw $t1, c
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t-1
	li $t0, 5
	sw $t0, t-1
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t0, x
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t0, x
	lw $t1, c
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t0, x
	lw $t1, a
	add $t2, $t0, $t1
	sw $t2, t-1
	li $t0, 5
	sw $t0, t-1
	lw $t0, x
	lw $t1, x
	add $t2, $t0, $t1
	sw $t2, t-1
	lw $t1, (null)
	move $t0, $t1
	sw $t0, y
	li $v0, 10
	syscall
