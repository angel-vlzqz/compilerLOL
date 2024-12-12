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
	la $t8, arr
	la $t1, t33
	lw $t0, 0($t1)
	sw $t0, 12($t8)
	li $t0, 10
	move $t1, $t0
	la $t2, 7.000000
	lw $t0, 0($t2)
	la $t3, 3.000000
	lw $t2, 0($t3)
	div $t0, $t2
	mflo $t3
	la $t0, 4.000000
	l.s $f0, 0($t0)
	mtc1 $t3, $f2
	cvt.s.w $f2, $f2
	mul.s $f4, $f0, $f2
	la $t0, 1.000000
	l.s $f0, 0($t0)
	add.s $f2, $f0, $f4
	cvt.w.s $f2, $f2
	mfc1 $t0, $f2
	move $t2, $t0
	li $t0, 4
	move $t3, $t0
	la $t8, arr
	la $t5, t42
	lw $t4, 0($t5)
	sw $t4, 16($t8)
	la $t5, scalar
	lw $t4, 0($t5)
	move $a0, $t4
	la $t8, arr
	lw $t5, 12($t8)
	move $a1, $t5
	la $t8, arr
	lw $t6, 16($t8)
	move $a2, $t6
	jal addAndMultiply
	la $t7, v0
	l.s $f0, 0($t7)
	mtc1 $t4, $f4
	cvt.s.w $f4, $f4
	div.s $f6, $f0, $f4
	li $t7, 1
