.data
x: .word 0
y: .word 0
z: .space 16
a: .word 0
b: .word 0
c: .word 0
t11: .word 0
t10: .word 0
t9: .word 0
t8: .word 0
90: .word 0
25: .word 0
9: .word 0
7: .word 0
2: .word 0
5: .word 0
1: .word 0
3: .word 0
0: .word 0
.text
.globl main
main:
# Generating MIPS code for array assignment
	li $t0, 0
	li $t1, 3
	la $t9, z
	mul $t0, $t0, 4
	add $t9, $t9, $t0
	sw $t1, 0($t9)
# Generating MIPS code for array assignment
	li $t2, 1
	li $t3, 5
	la $t9, z
	mul $t2, $t2, 4
	add $t9, $t9, $t2
	sw $t3, 0($t9)
# Generating MIPS code for array assignment
	li $t3, 2
	li $t4, 7
	la $t9, z
	mul $t3, $t3, 4
	add $t9, $t9, $t3
	sw $t4, 0($t9)
# Generating MIPS code for array assignment
	li $t4, 9
	la $t9, z
	mul $t1, $t1, 4
	add $t9, $t9, $t1
	sw $t4, 0($t9)
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
	la $t9, z
	mul $t0, $t0, 4
	add $t9, $t9, $t0
	lw $t4, 0($t9)
# Generating MIPS code for write operation
	move $a0, $t4
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for array access
	la $t9, z
	mul $t2, $t2, 4
	add $t9, $t9, $t2
	lw $t0, 0($t9)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for array access
	la $t9, z
	mul $t3, $t3, 4
	add $t9, $t9, $t3
	lw $t0, 0($t9)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for array access
	la $t9, z
	mul $t1, $t1, 4
	add $t9, $t9, $t1
	lw $t0, 0($t9)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	li $v0, 10
	syscall
