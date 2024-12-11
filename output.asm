.data
arr: .space 20
scalar: .float 0.0
x: .word 0
y: .word 0
z: .float 0.0
result: .float 0.0
i: .word 0
sum: .float 0.0
.text
main:
.globl main
# Prologue for function main
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
	lw $a0, scalar
	lw $a0, t23
	lw $a0, t24
# Function call to addAndMultiply
	jal addAndMultiply
# Epilogue for function main
main_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	li $v0, 10
	syscall

addAndMultiply:
	l.s $f12, z
	lw $a0, y
	lw $a0, x
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
