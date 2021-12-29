.intel_syntax noprefix

.data

.text

.equ spheres_count, 512
.equ sphereX, 0*spheres_count
.equ sphereY, 1*4*spheres_count
.equ sphereZ, 2*4*spheres_count
.equ sphereR, 3*4*spheres_count
.equ sphereM, 4*4*spheres_count
.equ sphereVecX, 5*4*spheres_count
.equ sphereVecY, 6*4*spheres_count
.equ sphereVecZ, 7*4*spheres_count
.equ sphereColor, 8*4*spheres_count

# void nbody_simd(t_sphere_array * _spheres RCX, int count RDX, float dt XMM2);
.global nbody_simd
nbody_simd:
	push rbp
	mov rbp, rsp
	and esp, 0xFFFFFFF0 # zarovnání stacku na 16 B

	# outer cycle (count on i = r15)
	mov r15, spheres_count

	# store dt from XMM2 in a GP register
	movq r8, xmm2

	# color green as a constant
	mov r9, 2
.outer:
	# inner cycle (count on j = r14 = i - 1)
	mov r14, r15
	dec r14

	# load and distribute spheres.xyzr[i] into XMM0–2, 6
	movss xmm0, [rcx + sphereX + r15 * 4 - 4] # XMM0 = spheres.x[i]
	shufps xmm0, xmm0, 0
	movss xmm1, [rcx + sphereY + r15 * 4 - 4] # XMM1 = spheres.y[i]
	shufps xmm1, xmm1, 0
	movss xmm2, [rcx + sphereZ + r15 * 4 - 4] # XMM2 = spheres.z[i]
	shufps xmm2, xmm2, 0

	movss xmm6, [rcx + sphereR + r15 * 4 - 4] # XMM6 = spheres.r[i]
	shufps xmm6, xmm6, 0

	# load and distribute spheres.vectorXYZ[i] into XMM11–13
	movss xmm11, [rcx + sphereVecX + r15 * 4 - 4] # XMM11 = spheres.vectorX[i]
	shufps xmm11, xmm11, 0
	movss xmm12, [rcx + sphereVecY + r15 * 4 - 4] # XMM12 = spheres.vectorY[i]
	shufps xmm12, xmm12, 0
	movss xmm13, [rcx + sphereVecZ + r15 * 4 - 4] # XMM13 = spheres.vectorZ[i]
	shufps xmm13, xmm13, 0
.inner:
	# inner cycle {
		# load spheres.xyz[j_] into XMM3–5, 7
		movups xmm3, [rcx + sphereX + r14 * 4 - 16] # XMM3 = spheres.x[j+3,j+2,j+1,j]
		movups xmm4, [rcx + sphereY + r14 * 4 - 16] # XMM4 = spheres.y[j_]
		movups xmm5, [rcx + sphereZ + r14 * 4 - 16] # XMM5 = spheres.z[j_]
		movups xmm7, [rcx + sphereR + r14 * 4 - 16] # XMM7 = spheres.r[j_]

		# XMM3 = spheres.x[j_] - spheres.x[i]  = XMM3 - XMM0
		# XMM8 = (spheres.x[i] - spheres.x[j_])^2 = (XMM3)^2
		subps xmm3, xmm0
		movaps xmm8, xmm3
		mulps xmm8, xmm8

		# XMM4 = spheres.y[j_] - spheres.y[i]  = XMM4 - XMM1
		# XMM9 = (spheres.y[i] - spheres.y[j_])^2 = (XMM4)^2
		subps xmm4, xmm1
		movaps xmm9, xmm4
		mulps xmm9, xmm9

		# XMM5 = spheres.z[j_] - spheres.z[i] = XMM5 - XMM2
		# XMM10 = (spheres.z[i] - spheres.z[j_])^2 = (XMM5)^2
		subps xmm5, xmm2
		movaps xmm10, xmm5
		mulps xmm10, xmm10

		# XMM8 = XMM8 + XMM9 + XMM10
		addps xmm8, xmm9
		addps xmm8, xmm10

		# XMM8 = sqrt(XMM8)
		sqrtps xmm8, xmm8

		# XMM7 = spheres.r[j_] + spheres.r[i] = XMM7 + XMM6
		addps xmm7, xmm6
		# XMM7 = compare XMM8, XMM7 = XMM7 >? sqrt(...)
		cmpps xmm7, xmm8, 6 # 6 = NLE

		# Compare sqrts to zero
		xorps xmm9, xmm9
		cmpps xmm9, xmm8, 4 # 4 = NEQ
		andps xmm7, xmm9

		# set color to green for all non-masked spheres
		movq xmm9, r9
		shufps xmm9, xmm9, 0
		vmaskmovps [rcx + sphereColor + r14 * 4 - 16], xmm7, xmm9

		# optimisation suggestion: check if all four are zero (PMOVMSKB) and if they are, end this cycle

		# divide axis-distances by distance to get norms
		divps xmm3, xmm8
		divps xmm4, xmm8
		divps xmm5, xmm8

		# state of registers now:
		# XMM0, XMM1, XMM2: base sphere [i] x, y, z distributed in all fields of the registers – CAN'T TOUCH THIS
		# XMM6: base sphere[i] r distributed – CAN'T TOUCH THIS
		# ----
		# XMM3: NEGATIVE n_x_norm, XMM4: NEGATIVE n_y_norm, XMM5: NEGATIVE n_z_norm (spheres.xyz[j] - spheres.xyz[i]) for the current spheres [j-3, j-2, j-1, j]
		# XMM7: mask for current [i] and [j-3, j-2, j-1, j]
		# XMM8–10: free to use
		# XMM11, XMM12, XMM13: base sphere [i] current vectorX, vectorY, vectorZ – should be updated in each cycle
		# XMM14, XMM15: free to use

		# load spheres.vectorXYZ[j_] into XMM8–10
		movups xmm8, [rcx + sphereVecX + r14 * 4 - 16] # XMM8 = spheres.vectorX[j+3,j+2,j+1,j]
		movups xmm9, [rcx + sphereVecY + r14 * 4 - 16] # XMM9 = spheres.vectorY[j_]
		movups xmm10, [rcx + sphereVecZ + r14 * 4 - 16] # XMM10 = spheres.vectorZ[j_]

		# make XMM14 an accumulator; use XMM15 for intermediate results
		movaps xmm14, xmm11
		mulps xmm14, xmm3

		movaps xmm15, xmm12
		mulps xmm15, xmm4
		addps xmm14, xmm15

		movaps xmm15, xmm13
		mulps xmm15, xmm5
		addps xmm14, xmm15

		# XMM14 now contains NEGATIVE a1_dot_
		movaps xmm15, xmm8
		mulps xmm15, xmm3
		subps xmm14, xmm15

		movaps xmm15, xmm9
		mulps xmm15, xmm4
		subps xmm14, xmm15

		movaps xmm15, xmm10
		mulps xmm15, xmm5
		subps xmm14, xmm15

		# XMM14 now contains (-a1_dot_ + a2_dot_) = -(a1_dot_ - a2_dot_)
		addps xmm14, xmm14

# debugging interrupt
#		cmp r15, 511
#		jne .abcd
#		cmp r14, 510
#		jne .abcd
#		int 3
#.abcd:

		movups xmm15, [rcx + sphereM + r15 * 4 - 4] 
		shufps xmm15, xmm15, 0

		# save xmm0 (general-purpose registers could be used here to improve perf)
		sub     esp, 16
		movdqu  oword ptr [esp], xmm0

		movups xmm0, [rcx + sphereM + r14 * 4 - 16]
		addps xmm15, xmm0

		# reload original xmm0
		movdqu  xmm0, oword ptr [esp]
		add     esp, 16

		divps xmm14, xmm15

		# XMM14 now contains NEGATIVE P_
		# We're going to add multiplies of P_, so now it's a good time to
		# use our mask to zero out the values in P that haven't passed 
		# the initial condition.

		andps xmm14, xmm7

		# XMM7 = spheres.m[j_] * (-P)
		movups xmm7, [rcx + sphereM + r14 * 4 - 16]
		mulps xmm7, xmm14

		# XMM15 = (spheres.m[j_] * (-P_)) * (-n_x_norm_)
		movaps xmm15, xmm7
		mulps xmm15, xmm3
		# spheres.vectorX[i] (XMM11) -= XMM15
		haddps xmm15, xmm15
		haddps xmm15, xmm15
		subps xmm11, xmm15

		# dtto for y, z
		movaps xmm15, xmm7
		mulps xmm15, xmm4
		haddps xmm15, xmm15
		haddps xmm15, xmm15
		subps xmm12, xmm15

		movaps xmm15, xmm7
		mulps xmm15, xmm5
		haddps xmm15, xmm15
		haddps xmm15, xmm15
		subps xmm13, xmm15

		# XMM7 = spheres.m[i] * (-P_)
		movups xmm7, [rcx + sphereM + r15 * 4 - 4]
		mulps xmm7, xmm14

		# XMM15 = (spheres.m[i] * (-P_)) * (-n_x_norm_)
		movaps xmm15, xmm7
		mulps xmm15, xmm3
		# spheres.vectorX[j_] (XMM8) += XMM15
		addps xmm8, xmm15

		# dtto for y, z
		movaps xmm15, xmm7
		mulps xmm15, xmm4
		addps xmm9, xmm15

		movaps xmm15, xmm7
		mulps xmm15, xmm5
		addps xmm10, xmm15

		# save back to memory
		movups [rcx + sphereVecX + r14 * 4 - 16], xmm8
		movups [rcx + sphereVecY + r14 * 4 - 16], xmm9
		movups [rcx + sphereVecZ + r14 * 4 - 16], xmm10

		sub r14, 4
		cmp r14, 4
		jle .inner_end
		jmp .inner
	# }
.inner_end:
	#int 3
	movss [rcx + sphereVecX + r15 * 4 - 4], xmm11
	movss [rcx + sphereVecY + r15 * 4 - 4], xmm12
	movss [rcx + sphereVecZ + r15 * 4 - 4], xmm13

	movq xmm15, r8
	mulss xmm11, xmm15
	mulss xmm12, xmm15
	mulss xmm13, xmm15
	addss xmm0, xmm11
	addss xmm1, xmm12
	addss xmm2, xmm13
	movss [rcx + sphereX + r15 * 4 - 4], xmm0
	movss [rcx + sphereY + r15 * 4 - 4], xmm1
	movss [rcx + sphereZ + r15 * 4 - 4], xmm2

	dec r15
	cmp r15, 4
	je .outer_end
	jmp .outer
.outer_end:

	mov rsp, rbp
	pop rbp
ret 0

