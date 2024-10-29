.data
a: .word 0
t0: .word 0
t1: .word 0
t2: .word 0
t3: .word 0
t4: .word 0
t5: .word 0
t6: .word 0
t7: .word 0
x: .word 0
y: .word 0
z: .space 16
floatA: .float 1.234000
angel: .word 0
adon: .word 0
b: .word 0
c: .word 0
.text
main:
.globl main
# Prologue for function main
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
# Generating MIPS code for assignment
	li $t0, 1
	move $t1, $t0
# Generating MIPS code for assignment
	li $t2, 0
	move $t3, $t2
# Generating MIPS code for assignment
	li $t4, 25
	move $t5, $t4
# Generating MIPS code for assignment
	li $t4, 90
	move $t6, $t4
# Generating MIPS code for floating point move
	mov.s $f2, $f0
# Generating MIPS code for array assignment
	la $t8, z
	li $t4, 3
	sw $t4, 0($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t7, 5
	sw $t7, 4($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t7, 7
	sw $t7, 8($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t7, 9
	sw $t7, 12($t8)
# Function call to print1
	jal print1
# Generating MIPS code for write operation
	move $a0, $t5
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	move $a0, $t6
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for array access
	la $t8, z
	lw $t5, 0($t8)
# Generating MIPS code for write operation
	move $a0, $t5
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t1 back to memory
	sw $t5, t1
# Generating MIPS code for array access
	la $t8, z
	lw $t2, 4($t8)
# Generating MIPS code for write operation
	move $a0, $t2
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t3 back to memory
	sw $t2, t3
# Generating MIPS code for array access
	la $t8, z
	lw $t0, 8($t8)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t5 back to memory
	sw $t0, t5
# Generating MIPS code for array access
	la $t8, z
	lw $t0, 12($t8)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t7 back to memory
	sw $t0, t7
# Generating MIPS code for write operation
	move $a0, $t1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	move $a0, $t3
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write_float operation
	mov.s $f12, $f2
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

print1:
# Prologue for function print1
	addiu $sp, $sp, -8
	sw $fp, 4($sp)
	sw $ra, 0($sp)
	move $fp, $sp
# Generating MIPS code for write operation
	li $t0, 1
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Epilogue for function print1
print1_epilogue:
	move $sp, $fp
	lw $fp, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 8
	jr $ra
