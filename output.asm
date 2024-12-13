.data
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
fcomp: .float 0.0
comp: .word 0
.text
main:
.globl main
# Prologue for function main
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
	li $t0, 7
	la $t8, arr
	sw $t0, 12($t8)
	li $t1, 10
	move $t2, $t1
	la $t3, 10.333332
	lw $t1, 0($t3)
	move $t3, $t1
	li $t1, 4
	move $t4, $t1
	li $t5, 7
	la $t8, arr
	sw $t5, 16($t8)
	la $t7, scalar
	lw $t6, 0($t7)
	move $a0, $t6
	li $t7, 7
	move $a1, $t7
	move $a2, $t7
	jal addAndMultiply
