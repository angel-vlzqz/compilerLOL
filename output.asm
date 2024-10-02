.data
y: .word 0
t27: .word 0
t26: .word 0
t25: .word 0
t24: .word 0
t21: .word 0
t20: .word 0
t19: .word 0
t17: .word 0
t16: .word 0
t14: .word 0
t10: .word 0
b: .word 0
.text
.globl main
main:
	li $t0, 12
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t10
	li $t0, 1
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t14
	li $t0, 1
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t16
	lw $t0, t16
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t17
	li $t0, 1
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t19
	lw $t0, t19
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t20
	lw $t0, t20
	li $t1, 3
	add $t2, $t0, $t1
	sw $t2, t21
	li $t0, 1
	li $t1, 12
	add $t2, $t0, $t1
	sw $t2, t24
	lw $t0, t24
	lw $t1, b
	add $t2, $t0, $t1
	sw $t2, t25
	lw $t0, t25
	li $t1, 3
	add $t2, $t0, $t1
	sw $t2, t26
	lw $t0, t26
	li $t1, 12
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
