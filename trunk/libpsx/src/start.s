# This is the start code for the PSXSDK.
# It sets needed things up, and calls the setup and the main function.

# This has to be linked in as the first object when using ld, so that it
# appears at the start of the .text section.

	.align 16
	.text
.global _start	
.global vblank_handler
.extern vblank_handler_callback

.global rcnt_handler
.extern rcnt_handler_callback

.global get_cop0_status
.global set_cop0_status
.global get_cop0_epc
.global get_gp
.global run_bios
	
_start:
#	la $28, _gp # Load Global Pointer value
	li $29, 0x801fff00 # Load stack pointer
	li $k1, 0x1f800000            # set to hardware base
	
# This has to be replaced by a jal, sometime...
	
	jal ResetEntryInt
	nop

# psxsdk_setup: This might make things not run on PSOne...
	
	jal psxsdk_setup
	nop
	
	j main
	nop
	
inf_loop:
	j inf_loop
	nop

# VBlank handler

vblank_handler:
	addi $sp, -112
	sw $at, 0($sp)
	sw $v0, 4($sp)
	sw $v1, 8($sp)
	sw $a0, 12($sp)
	sw $a1, 16($sp)
	sw $a2, 20($sp)
	sw $a3, 24($sp)
	sw $t0, 28($sp)
	sw $t1, 32($sp)
	sw $t2, 36($sp)
	sw $t3, 40($sp)
	sw $t4, 44($sp)
	sw $t5, 48($sp)
	sw $t6, 52($sp)
	sw $t7, 56($sp)
	sw $s0, 60($sp)
	sw $s1, 64($sp)
	sw $s2, 68($sp)
	sw $s3, 72($sp)
	sw $s4, 76($sp)
	sw $s5, 80($sp)
	sw $s6, 84($sp)
	sw $s7, 88($sp)
	sw $t8, 92($sp)
	sw $t9, 96($sp)
	sw $gp, 100($sp)
	sw $s8, 104($sp)
	sw $ra, 108($sp)
	
vblank_fire_user_handler:

	la $t0, vblank_handler_callback
	lw $t1, 0($t0)
	
	jalr $t1
	nop
	
vblank_handler_end:
	lw $at, 0($sp)
	lw $v0, 4($sp)
	lw $v1, 8($sp)
	lw $a0, 12($sp)
	lw $a1, 16($sp)
	lw $a2, 20($sp)
	lw $a3, 24($sp)
	lw $t0, 28($sp)
	lw $t1, 32($sp)
	lw $t2, 36($sp)
	lw $t3, 40($sp)
	lw $t4, 44($sp)
	lw $t5, 48($sp)
	lw $t6, 52($sp)
	lw $t7, 56($sp)
	lw $s0, 60($sp)
	lw $s1, 64($sp)
	lw $s2, 68($sp)
	lw $s3, 72($sp)
	lw $s4, 76($sp)
	lw $s5, 80($sp)
	lw $s6, 84($sp)
	lw $s7, 88($sp)
	lw $t8, 92($sp)
	lw $t9, 96($sp)
	lw $gp, 100($sp)
	lw $s8, 104($sp)
	lw $ra, 108($sp)
	addi $sp, 112
	jr $ra
	nop

# Root counter handler

rcnt_handler:
	addi $sp, -112
	sw $at, 0($sp)
	sw $v0, 4($sp)
	sw $v1, 8($sp)
	sw $a0, 12($sp)
	sw $a1, 16($sp)
	sw $a2, 20($sp)
	sw $a3, 24($sp)
	sw $t0, 28($sp)
	sw $t1, 32($sp)
	sw $t2, 36($sp)
	sw $t3, 40($sp)
	sw $t4, 44($sp)
	sw $t5, 48($sp)
	sw $t6, 52($sp)
	sw $t7, 56($sp)
	sw $s0, 60($sp)
	sw $s1, 64($sp)
	sw $s2, 68($sp)
	sw $s3, 72($sp)
	sw $s4, 76($sp)
	sw $s5, 80($sp)
	sw $s6, 84($sp)
	sw $s7, 88($sp)
	sw $t8, 92($sp)
	sw $t9, 96($sp)
	sw $gp, 100($sp)
	sw $s8, 104($sp)
	sw $ra, 108($sp)

rcnt_fire_user_handler:

	la $t0, rcnt_handler_callback
	lw $t1, 0($t0)
	
	jalr $t1
	nop
	
rcnt_handler_end:
	lw $at, 0($sp)
	lw $v0, 4($sp)
	lw $v1, 8($sp)
	lw $a0, 12($sp)
	lw $a1, 16($sp)
	lw $a2, 20($sp)
	lw $a3, 24($sp)
	lw $t0, 28($sp)
	lw $t1, 32($sp)
	lw $t2, 36($sp)
	lw $t3, 40($sp)
	lw $t4, 44($sp)
	lw $t5, 48($sp)
	lw $t6, 52($sp)
	lw $t7, 56($sp)
	lw $s0, 60($sp)
	lw $s1, 64($sp)
	lw $s2, 68($sp)
	lw $s3, 72($sp)
	lw $s4, 76($sp)
	lw $s5, 80($sp)
	lw $s6, 84($sp)
	lw $s7, 88($sp)
	lw $t8, 92($sp)
	lw $t9, 96($sp)
	lw $gp, 100($sp)
	lw $s8, 104($sp)
	lw $ra, 108($sp)
	addi $sp, 112
	jr $ra
	nop

get_cop0_status:
	mfc0 $2, $12
	jr $ra
	nop

set_cop0_status:
	mtc0 $4, $12
	jr $ra
	nop
	
get_cop0_epc:
	mfc0 $2, $14
	jr $ra
	nop

get_gp:
	add $2, $zero, $fp
	jr $ra
	nop

run_bios:
	li $8, 0xbfc00000
	jr $8
	nop
	
