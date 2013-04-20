 ;
 ; This file is part of Badadroid project.
 ;
 ; Copyright (C) 2012 Rebellos, mijoma, b_kubica
 ;
 ;
 ; Badadroid is free software: you can redistribute it and/or modify
 ; it under the terms of the GNU General Public License as published by
 ; the Free Software Foundation, either version 3 of the License, or
 ; (at your option) any later version.
 ;
 ; Badadroid is distributed in the hope that it will be useful,
 ; but WITHOUT ANY WARRANTY; without even the implied warranty of
 ; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ; GNU General Public License for more details.
 ;
 ; You should have received a copy of the GNU General Public License
 ; along with Badadroid.  If not, see <http://www.gnu.org/licenses/>.
 ;
 ;
 
; useful data size units
KB equ * 1024
MB equ * 1024 KB
GB equ * 1024 MB

; endless loop
NORETURN fix b $

; prevent using macro more than once
macro include_once name
{
macro name {
	if ~defined name#_INCLUDED
	name#_INCLUDED equ 1

	align 4
}
INCLUDE_ONCE fix include_once
END_INCLUDE_ONCE fix end if }


macro START
{
	format binary as 'fota'

	processor 0x2FFFFFE
	coprocessor 0x30F8

	org FOTA_DRAM_START
	align 4

c_start:

	b	c_run

f_ver:
	db 0x100 - ( f_ver - c_start ) dup 0
	db 'FOTA_ENGINE_VER_INFO_2.0'
	db 232 dup 0
	dw FOTA_DRAM_START			; not required for < 2.0
	db 252 dup 0				; not required for < 2.0

c_run:
	str		lr, [bl_return_addr]	; preserve return address

	ldr		r0, [_key2]
	ldrb		r0, [r0, 0]		; Menu Key
	cmp		r0, 0
	ldrne		r0, [s_atagcmdlineRM_a] ; quick and dirty address copy...
	ldrne		r1, [s_atagcmdline_aa]
	strne		r0, [r1]
	bne		fota_run		; recovery mode

	ldr		r0, [_key2]
	ldrb		r0, [r0, 7]		; VolUP Key
	cmp		r0, 0
	ldrne		r0, [s_atagcmdlineRM_b] ; quick and dirty address copy...
	ldrne		r1, [s_atagcmdline_aa]
	strne		r0, [r1]
	bne		fota_run		; bigmem


	ldr		r0, [_key2]
	ldrb		r0, [r0, 2]
	cmp		r0, 0
	beq		fota_run		    ; not menu/home, not vol-up, and not call-key   - android


return_to_bl:
	ldr	pc, [bl_return_addr]		 ; back to the bootloader

fota_dload_mode:
	MOV	R0, 1
	ADR	R1, used_download
	STR	R0, [R1]
	bl		enable_fota_output
	bl		dloadmode

	NORETURN

	align 4
	bl_return_addr				dw 0x0 ; filled in runtime
	_key2					dw pressed_button
	s_atagcmdlineRM_a			dw s_atagcmdlineRM
	s_atagcmdline_aa			dw s_atagcmdline_a
	s_atagcmdline_a 			dw s_atagcmdline
	s_atagcmdlineRM_b			dw s_atagcmdlineRMb

	used_download				dw 0x0

	s_atagcmdlineRM 			db 'bootmode=2 loglevel=0',0
	s_atagcmdline				db 'loglevel=0',0
	s_atagcmdlineRMb			db 'bootmode=10 loglevel=0',0

	align 4

fota_run:
}

macro END
{

magic_1:
	code_len = magic_1 - c_start
	db 0x280000 - code_len dup 0xFF
	db 'BPDZ'
	db 12 dup 0
	dw 0xABCDABCD
	dw FOTA_NAND_START
	dw 0
	db PRODUCT_CODE
	db 27 dup 0
	db 'fota'
	dw 0
	dw 6
	dw 1
	dw 2
	dw 0x800
	dw 0x20000

magic_2:
	code_len = magic_2 - c_start
	db 0x280410 - code_len dup 0
}
