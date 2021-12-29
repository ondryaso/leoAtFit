.intel_syntax noprefix

.data


	ballXmin: .float 0.0
	ballYmin: .float 0.0
	ballXmax: .float 0.0
	ballYmax: .float 0.0

	ballY:	.float 0.000
	step:	.float 0.001
	clearx:	.float 0.0


.text

.global update
update:
    push rbp
    mov rbp, rsp

    mov rsp, rbp
    pop rbp
    ret



.global getBallYCord
getBallYCord:
    push rbp
    mov rbp, rsp

    mov rsp, rbp
    pop rbp
    ret
