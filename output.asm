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
	li $t0, 10
	move $t1, $t0
# Integer arithmetic /
	lw $t0, 7.000000
	lw $t2, 3.000000
	div $t0, $t2
	mflo $t3
	l.s $f0, 4.000000
	mtc1 $t3, $f2
	cvt.s.w $f2, $f2
	mul.s $f4, $f0, $f2
	l.s $f0, 1.000000
	add.s $f2, $f0, $f4
# Assignment
	cvt.w.s $f2, $f2
	mfc1 $t0, $f2
	move $t2, $t0
# Assignment
	li $t0, 4
	move $t3, $t0
# Array store
	la $t8, arr
	lw $t4, t42
	sw $t4, 16($t8)
	lw $t4, scalar
	move $a0, $t4
# Array load
	la $t8, arr
	lw $t5, 12($t8)
	move $a1, $t5
# Array load
	la $t8, arr
	lw $t5, 16($t8)
	move $a2, $t5
# Function call to addAndMultiply
	jal addAndMultiply
	l.s $f0, v0
	mtc1 $t4, $f4
	cvt.s.w $f4, $f4
	div.s $f6, $f0, $f4
# Integer arithmetic -
	li $t5, 1
	sub $t6, $t3, $t5
# Array load
	la $t8, arr
	mul $t9, $t6, 4
	add $t9, $t8, $t9
	lw $t5, 0($t9)
	mtc1 $t5, $f8
	cvt.s.w $f8, $f8
	mul.s $f10, $f8, $f4
	add.s $f8, $f6, $f10
# Array load
	la $t8, arr
	mul $t9, $t3, 4
	add $t9, $t8, $t9
	lw $t5, 0($t9)
	mtc1 $t5, $f6
	cvt.s.w $f6, $f6
	add.s $f10, $f6, $f4
# Write float
	mov.s $f12, $f0
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Write float
	l.s $f0, t54
	mov.s $f12, $f0
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Write float
	mov.s $f12, $f4
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	sw $t4, scalar
	s.s $f4, scalar
# Write float
	l.s $f0, scalar2
	mov.s $f12, $f0
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	s.s $f0, scalar2
# Write integer
	lw $t3, scalar3
	move $a0, $t3
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
	sw $t3, scalar3
# Array load
	la $t8, arr
	lw $t3, 16($t8)
# Write integer
	move $a0, $t3
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Write integer
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Write float
	mtc1 $t2, $f0
	cvt.s.w $f0, $f0
	mov.s $f12, $f0
	li $v0, 2
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Epilogue for function main
main_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	li $v0, 10
	syscall

addAndMultiply:
	l.s $f0, z
	mov.s $f12, $f0
	lw $t0, y
	move $a1, $t0
	lw $t0, x
	move $a2, $t0
# Prologue for function addAndMultiply
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
# Epilogue for function addAndMultiply
addAndMultiply_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	jr $ra
