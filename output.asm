.data
spill_area: .word 0
main: .word 0
.text
.globl main
main:
# Generating MIPS code for floating-point assignment
	mov.s $f2, $f0
# Generating MIPS code for array assignment
	la $t8, z
# Spilling register $t0 to memory
	sw $t0, spill_area
	li $t0, 3
	sw $t0, 0($t8)
# Generating MIPS code for array assignment
	la $t8, z
# Spilling register $t0 to memory
	sw $t0, spill_area
	li $t0, 5
	sw $t0, 4($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t0, 7
	sw $t0, 8($t8)
# Generating MIPS code for array assignment
	la $t8, z
	li $t0, 9
	sw $t0, 12($t8)
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
	la $t8, z
	lw $t0, 0($t8)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t1 back to memory
	sw $t0, t1
# Generating MIPS code for array access
	la $t8, z
	lw $t0, 4($t8)
# Generating MIPS code for write operation
	move $a0, $t0
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Storing variable t3 back to memory
	sw $t0, t3
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
	li $a0, 1
	li $v0, 1
	syscall
	li $a0, 10
	li $v0, 11
	syscall
# Generating MIPS code for write operation
	li $a0, 0
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
	li $v0, 10
	syscall
