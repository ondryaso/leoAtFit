.intel_syntax noprefix

.data
	constants: .byte 50,50,50,50,50,50,50,50


.text

.global _DllMainCRTStartup
_DllMainCRTStartup:
	push rbp
	mov rbp, rsp
	
	mov eax, ebx
	mov rax,1
	movq rsp, rbp
	pop rbp
ret 


.global test
test:

ret 0

# (*Brightness)(unsigned char *input_data RCX, unsigned char *output_data RDX, unsigned int width R8, unsigned int height R9, int argc STACK, char **argv STACK);
.global brightness
brightness:
 	# store rdx (will be overwritten by MUL)
	mov r10, rdx 
	# calculate total size of data (w*h*3)
	mov rax, 3  
	mul r8
	mul r9
	# loop
	mov rcx, rax
.div_b_loop:
	mov al, byte ptr [r10 + rcx - 1]
	add al, byte ptr [rip + constants]

	jnc .cont2
	mov al, 255
.cont2:
	mov byte ptr [r10 + rcx - 1], al

	loop .div_b_loop

ret 0

.global brightness_mmx
brightness_mmx:
 	# store rdx (will be overwritten by MUL)
	mov r10, rdx 
	# calculate total size of data (w*h*3/8)
	mov rax, 3  
	mul r8
	mul r9

	mov rbx, 8
	div rbx

	movq xmm2, qword ptr [rip + constants]
	xor rcx, rcx
.div_b_loop_2:
	cmp rcx, rax
	je .l_end
	
	movq xmm1, qword ptr [r10 + rcx*8]
	paddusb xmm1, xmm2
	movq qword ptr [r10 + rcx*8], xmm1

	inc rcx
	jmp .div_b_loop_2
.l_end:

ret 0

