

;   0x02000 [dword] Memory Size
;   0x02004 [dword] ptable addr
;   0x03000 page table
;
;   0x80000 reloc addr
;
;   0x90000 prot mode targ 
;   0x93FFF

memamt equ 0x2000
ptaddr equ 0x2004
entry equ 0x90000
romsize equ 0x4000

	bits 16

rom_start:	
	dw 0xaa55						; rom signature
	db 0x20							; rom size (512 byte units)
	jmp short s1					; entrypoint
	db 0x42							; checksum (dummy)
	
	times 0x18 - ($ - rom_start) db 0

	dw pci_header					; PCI Header Offset
	dw 0x0000						; PNP Gunk

pci_header:
	db 'PCIR'
	dw 0x10ec						; vendor
	dw 0x8029						; device
	dw 0x0000						; vital data
	dw 0x0018						; length
	db 0							; pci_header version (0)						
	db 0							; protocol
	db 0							; subclass (ethernet)
	db 2							; class (network)
	dw 0x0000						; image length
	dw 0x0000						; revision
	db 0							; code type (x86)
	db 0x80							; indicates this is last image
	dw 0x0000						; reserved

s1:
	push ds							; hook into the boot sequence
	
	xor ax, ax
	mov ds, ax
	
	mov word [0x19 * 4], start
	mov word [0x19 * 4 + 2], cs
	
	pop ds
	retf
	
start:
	cli
	cld

	xor di,di						; relocate to 8000:0000
	xor si,si

	mov ax,0x8000
	mov es,ax

	mov cx,romsize/4
	cs
	rep
	movsd	
	
	xor ax,ax
	mov ss,ax
	mov sp,0xfffc
	
	mov ax,0x8000	
	mov ds,ax

	jmp 0x8000:start_reloc

start_reloc:

;	mov ax, 0x4f02						; VESA Gunk
;	mov bx, (0x8000 + 0x4000 + 0x105)
;	int 0x10

	call enableA20
	
	lgdt [unGDT]
	
	mov eax,cr0
	or al, 1
	mov cr0, eax

;	jump dword 0x8:(0x80000 + setsegs)
	db 0x66, 0xea
	dw setsegs
	dw 0x0008
	dw 0x0008
	
setsegs:
	mov bx,0x10
	mov ds,bx
	mov es,bx
	mov ss,bx
	
	and al,0xfe
	mov cr0, eax
	
	jmp 0x8000:newip
		
newip:
	xor ax,ax
	mov es,ax
	mov ds,ax
	mov ss,ax
	
	mov edi,0x90000				; relocate rom to just below 1MB
	mov esi, eof + 0x80000
	mov ecx, romsize/4
	rep a32 movsd

	call countmemory

	mov dword [memamt], eax
	mov dword [ptaddr], 0x3000

; make some pages tables
	mov ecx, eax
	shr ecx, 12
	mov edi, [ptaddr]
	add ecx, 0x1000/4
	
	xor eax, eax
	rep a32 stosd
	
	mov ebx, [ptaddr]
	mov ecx, [memamt]
	
	shr ecx, 12
	mov eax, 0x0003
	mov edi, 0x1000
	
l1:
	mov [ebx+edi],eax
	add eax, 0x1000
	add edi, 4
	loop l1
	
	mov ecx, [memamt]
	shr ecx, 22
	inc ecx
	
	mov eax, [ptaddr]
	add eax, 0x1003
	xor edi, edi
	
l2:
	mov [ebx+edi], eax
	mov [ebx+edi+0xf00], eax
	add eax, 0x1000
	add edi, 4
	loop l2
	

	mov eax, [ptaddr]
	mov ecx, eax
	or al, 3
	
	mov [ebx+0xffc], eax
	
	mov ebx, 0x90000
	
goprot:
	push dword 0x0002
	popfd
	
	lgdt [cs:GDT]	

	mov cr3, ecx
	mov eax, 0x80000001
	mov cr0, eax

;	jmp far prot
	db 0x66, 0xea
	dw 0x0000 + prot, 0x0008, 0x0008
	
prot:
	bits 32
	
	mov eax, 0x10
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	
	mov esp, ecx
	sub esp, 4
	
	push dword [memamt]		; memory amount
	
	call ebx
	jmp $
	
	align 8
	
unGDT:
	dw 0xffff, unGDT + 0x0000, 0x0008, 0x0000			; offset unGDT
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0x8f, 0x00	; kCS (0x08) CPL0
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0x8f, 0x00	; kDS (0x10)
	
GDT:
	dw 0xffff, GDT + 0x0000, 0x0008, 0x0000				; offset GDT
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00	; kCS (0x08)
	db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00	; kDS (0x10)

	bits 16	

enableA20:
	call enableA20o1
	jnz short enableA20done
	mov al,0xd1
	out 0x64,al
	call enableA20o1
	jnz short enableA20done
	mov al,0xdf
	out 0x60,al
enableA20o1:
	mov ecx,0x20000
enableA20o1l:
	jmp short enableA20next
enableA20next:
	in al,0x64
	test al,2
	loopnz enableA20o1l
enableA20done:
	ret
	
countmemory:
	mov ax, '12'
	mov ebx, 0x100ff0
	
b0:
	mov dx, [ebx]			; save contents
	mov [ebx], ax			; write signature
	
	mov di, [ebx]			; read back
	mov [ebx], dx			; restore contents
	
	cmp di, ax				; did it stick?
	jnz b1					; nope, no more memory
	
	add ebx, 0x1000
	jmp b0
	
b1:
	mov eax, ebx
	sub eax, 0x1000
	add eax, 0x10
	
	ret
	
eof:
