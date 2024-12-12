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
# Array store
	la $t8, arr
	lw $t0, t33
	sw $t0, 12($t8)
# Assignment
	li $t1, 10
	move $t2, $t1
# Integer arithmetic /
	lw $t3, 7.000000
	lw $t4, 3.000000
	div $t3, $t4
	mflo $t5
# Assignment
	lw $t6, t39
	move $t7, $t6
# Assignment
