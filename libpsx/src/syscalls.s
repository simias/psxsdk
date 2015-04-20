# PSX SDK System calls file

.text

.global ReadSector

ReadSector:
	li $9, 0xa5
	j 0xa0
	nop
	jr $ra
	nop

# Console functions

.global putchar
.global puts
.global printf

putchar:
	li $9, 0x3c
	j 0xa0
	nop
	jr $ra
	nop
	
puts:
	li $9, 0x3e
	j 0xa0
	nop
	jr $ra
	nop
printf:
	li $9, 0x3f
	j 0xa0
	nop
	jr $ra
	nop

# Conversion functions

#.global atoi
#.global atol

#atoi:
#	li $9, 0x10
#	j 0xa0
#	nop
#	jr $ra
#	nop
#atol:
#	li $9, 0x11
#	j 0xa0
#	nop
#	jr $ra
#	nop
#atob:
#	li $9, 0x12
#	j 0xa0
#	nop
#	jr $ra
#	nop

# String functions

#.global strcat
#.global strncat
#.global strcmp
#.global strncmp
#.global strcpy
#.global strncpy
#.global strlen
#.global index
#.global rindex
#.global strchr
#.global strrchr
#.global strpbrk
#.global strspn
#.global strcspn
#.global strtok
#.global strstr

#strcat:
#	li $9, 0x15
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#strncat:
#	li $9, 0x16
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#strcmp:
#	li $9, 0x17
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#strncmp:
#	li $9, 0x18
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strcpy:
#	li $9, 0x19
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strncpy:
#	li $9, 0x1a
#	j 0xa0
#	jr $ra
#strlen:
#	li $9, 0x1b
#	j 0xa0
#nop
#	jr $ra
#	nop
#index:
#	li $9, 0x1c
#	j 0xa0
#	nop
#	jr $ra
#	nop
#rindex:
#	li $9, 0x1d
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strchr:
#	li $9, 0x1e
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strrchr:
#	li $9, 0x1f
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strpbrk:
#	li $9, 0x20
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strspn:
#	li $9, 0x21
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strcspn:
#	li $9, 0x22
#	j 0xa0
#	nop
#	jr $ra
#     nop
#strtok:
#	li $9, 0x23
#	j 0xa0
#	nop
#	jr $ra
#	nop
#strstr:
#	li $9, 0x24
#	j 0xa0
#	nop
#	jr $ra
#	nop

# Misc functions
.global toupper
.global tolower
.global rand
.global srand
.global qsort

toupper:
	li $9, 0x25
	j 0xa0
	nop
	jr $ra
	nop
	
tolower:
	li $9, 0x26
	j 0xa0
	nop
	jr $ra
	nop
	
rand:
	li $9, 0x2f
	j 0xa0
	nop
	jr $ra
	nop
	
srand:
	li $9, 0x30
	j 0xa0
	nop
	jr $ra
	nop

qsort:
	li $9, 0x31
	j 0xa0
	nop
	jr $ra
	nop

# Memory functions
#.global bcopy
#.global bzero
#.global bcmp
#.global memcpy
#.global memset
#.global memmove
# .global memchr
#.global malloc
#.global free
.global lsearch
.global bsearch
#.global calloc
#.global realloc
.global InitHeap
.global FlushCache

#bcopy:
#	li $9, 0x27
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#bzero:
#	li $9, 0x28
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#bcmp:
#	li $9, 0x29
#	j 0xa0
#	nop
#	jr $ra
#	nop
		
#memset:
#	li $9, 0x2b
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#memmove:
#	li $9, 0x2c
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#memcpy:
#	li $9, 0x2d
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#memchr:
#	li $9, 0x2e
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#malloc:
#	li $9, 0x33
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#free:
#	li $9, 0x34
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#lsearch:
#	li $9, 0x35
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#bsearch:
#	li $9, 0x36
#	j 0xa0
#	nop
#	jr $ra
#	nop

#calloc:
#	li $9, 0x37
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
#realloc:
#	li $9, 0x38
#	j 0xa0
#	nop
#	jr $ra
#	nop
	
InitHeap:
	li $9, 0x39
	j 0xa0
	nop
	jr $ra
	nop
	
FlushCache:
	li $9, 0x44
	j 0xa0
	nop
	jr $ra
	nop

# GPU functions
.global GPU_dw
.global mem2vram
.global SendGPU
.global GPU_cw
.global GPU_cwb
.global SendPrimitive
.global GetGPUStatus

GPU_dw:
	li $9, 0x46
	j 0xa0
	nop
	jr $ra
	nop

mem2vram:
	li $9, 0x47
	j 0xa0
	nop
	jr $ra
	nop

# Joypad functions
.global PAD_init
.global PAD_dr
.global InitPAD
.global StartPAD
.global StopPAD
.global ChangeClearPAD
.global ResetEntryInt

PAD_init:
	li $9, 0x15
	li $10, 0xb0
	jr $10
	nop
	jr $ra
	nop
	
PAD_dr:
	li $9, 0x16
	li $10, 0xb0
	jr $10
	nop
	jr $ra
	nop

InitPAD:
	li $9, 0x12
	j 0xb0
	nop
	jr $ra
	nop
	
StartPAD:
	li $9, 0x13
	j 0xb0
	nop
	jr $ra
	nop
	
StopPAD:
	li $9, 0x14
	j 0xb0
	nop
	jr $ra
	nop
	
ChangeClearPAD:
	li $9, 0x5b
	j 0xb0
	nop
	jr $ra
	nop

ResetEntryInt:
	li $9, 0x18
	j 0xb0
	nop
	jr $ra
	nop

# ???
.global GetKernelDate
.global GetKernelRomVersion
.global GetRamSize

GetKernelDate:
	li $9, 0xb4
	li $4, 0
	j 0xa0
	nop
	jr $ra
	nop

GetKernelRomVersion:
	li $9, 0xb4
	li $4, 2
	j 0xa0
	nop
	jr $ra
	nop

GetRamSize:
	li $9, 0xb4
	li $4, 5
	j 0xa0
	nop
	jr $ra
	nop

# Event functions
.global OpenEvent
.global EnableEvent
.global CloseEvent
.global DisableEvent
.global WaitEvent
.global TestEvent
.global DeliverEvent

OpenEvent:
	li $9, 0x08
	j 0xb0
	nop
	jr $ra
	nop

EnableEvent:
	li $9, 0x0c
	j 0xb0
	nop
	jr $ra
	nop

CloseEvent:
	li $9, 0x09
	j 0xb0
	nop
	jr $ra
	nop
	
DisableEvent:
	li $9, 0x0d
	j 0xb0
	nop
	jr $ra
	nop

DeliverEvent:
	li $9, 0x07
	j 0xb0
	nop
	jr $ra
	nop
	
WaitEvent:
	li $9, 0x0a
	j 0xb0
	nop
	jr $ra
	nop
	
TestEvent:
	li $9, 0x0b
	j 0xb0
	nop
	jr $ra
	nop

# File I/O functions
.global open
.global lseek
.global read
.global write
.global close
.global cd
.global firstfile
.global nextfile
.global rename
.global delete

open:
	li $9, 0x32
	j 0xb0
	nop
	jr $ra
	nop
	
lseek:
	li $9, 0x33
	j 0xb0
	nop
	jr $ra
	nop
	
read:
	li $9, 0x34
	j 0xb0
	nop
	jr $ra
	nop
	
write:
	li $9, 0x35
	j 0xb0
	nop
	jr $ra
	nop
	
close:
	li $9, 0x36
	j 0xb0
	nop
	jr $ra
	nop
	
cd:
	li $9, 0x40
	j 0xb0
	nop
	jr $ra
	nop
	
firstfile:
	li $9, 0x42
	j 0xb0
	nop
	jr $ra
	nop
	
nextfile:
	li $9, 0x43
	j 0xb0
	nop
	jr $ra
	nop
	
rename:
	li $9, 0x44
	j 0xb0
	nop
	jr $ra
	nop
	
delete:
	li $9, 0x45
	j 0xb0
	nop
	jr $ra
	nop
	
# Exception / Interrupt functions

#.global Exception
.global EnterCriticalSection
.global ExitCriticalSection
.global SysEnqIntRP
.global SysDeqIntRP

#Exception:
#	li $a0, 0
#	syscall
#	nop
#	jr $ra
#	nop
	
EnterCriticalSection:
	li $a0, 1
	#syscall
	nop
	jr $ra
	nop

ExitCriticalSection:
	li $a0, 2
	#syscall
	nop
	jr $ra
	nop

SysEnqIntRP:
	li $9, 0x02
	j 0xc0
	nop
	jr $ra
	nop
	
SysDeqIntRP:
	li $9, 0x03
	j 0xc0
	nop
	jr $ra
	nop
	
# Filesystem functions

.global _96_init
.global _96_remove
.global _bu_init

_96_init:
	li $9, 0x71
	j 0xa0
	nop
	jr $ra
	nop
	
_96_remove:
	li $9, 0x72
	j 0xa0
	nop
	jr $ra
	nop
	
_bu_init:
	li $9, 0x70
	j 0xa0
	nop
	jr $ra
	nop
	
# Executable loading functions

.global LoadExec

LoadExec:
	li $9, 0x51
	j 0xa0
	nop
	jr $ra
	nop

# Memory card routines

.global InitCARD
.global StartCARD
.global StopCARD
.global _card_info
.global _card_load
.global _card_auto
.global _card_write
.global _card_read
.global _card_status
.global _new_card

InitCARD:
	li $9, 0x4a
	j 0xb0
	nop
	jr $ra
	nop
	
StartCARD:
	li $9, 0x4b
	j 0xb0
	nop
	jr $ra
	nop

StopCARD:
	li $9, 0x4c
	j 0xb0
	nop
	jr $ra
	nop

_card_info:
	li $9, 0xab
	j 0xa0
	nop
	jr $ra
	nop
	
_card_load:
	li $9, 0xac
	j 0xa0
	nop
	jr $ra
	nop
	
_card_auto:
	li $9, 0xad
	j 0xa0
	nop
	jr $ra
	nop

_card_write:
	li $9, 0x4e
	j 0xb0
	nop
	jr $ra
	nop
	
_card_read:
	li $9, 0x4f
	j 0xb0
	nop
	jr $ra
	nop

_new_card:
	li $9, 0x50
	j 0xb0
	nop
	jr $ra
	nop
	
_card_status:
	li $9, 0x5c
	j 0xb0
	nop
	jr $ra
	nop

# Device functions

.global PrintInstalledDevices

PrintInstalledDevices:
	li $9, 0x49
	j 0xb0
	nop
	jr $ra
	nop
