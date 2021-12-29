.intel_syntax noprefix


.data

	G: .float 9.8
	constant_05: .float 0.5
	constant_160: .float -160.0
	constant_160_vec:  .float -160.0,-160.0,-160.0,-160.0,-160.0,-160.0,-160.0,-160.0
.text

	.equ SPHERES_COUNT, 4096
	.equ spheres.x,0
	.equ spheres.y,SPHERES_COUNT*4
	.equ spheres.z,SPHERES_COUNT*2*4
	.equ spheres.r,SPHERES_COUNT*3*4
	.equ spheres.v,SPHERES_COUNT*4*4
	.equ spheres.q,SPHERES_COUNT*5*4


# void BouncingBall(t_sphere_array *_spheres RCX, int count RDX, float dt XMM2)
.global BouncingBall_ASM
BouncingBall_ASM:
push rbp
mov rbp, rsp
and esp, 0xFFFFFFF0

# YMM1 = G
# YMM2 = dt
# YMM3 = G*dt
# YMM4 = 0.5*G*dt
# YMM8 = const -160
vbroadcastss ymm1, dword ptr [rip + G]
vbroadcastss ymm2, xmm2
vmulps ymm3, ymm1, ymm2
vbroadcastss ymm4, dword ptr [rip + constant_05]
vmulps ymm4, ymm4, ymm3

vmovups ymm8, [rip + constant_160_vec]

# count on RDX = i
.cycle:
# YMM1 = spheres.y[i_]
# YMM2 = dt
# YMM3 = G*dt
# YMM4 = 0.5*G*dt
# YMM5 = spheres.q[i_]
# YMM6 = spheres.v[i_]
vmovups ymm1, [rcx + spheres.y + rdx * 4 - 32]
vmovups ymm5, [rcx + spheres.q + rdx * 4 - 32]
vmovups ymm6, [rcx + spheres.v + rdx * 4 - 32]

# calculate new Y (result in YMM1)
vsubps ymm7, ymm6, ymm4
vmulps ymm7, ymm7, ymm2
vsubps ymm1, ymm1, ymm7

# calculate new speed (result in YMM6)
vaddps ymm6, ymm6, ymm3

# compare y_new (YMM1) to -160 (const in ymm8), mask in YMM9
vcmpps ymm9, ymm1, ymm8, 1 #(1=LT)

# (if y<-160) spheres.y[i_] = -160 (const in ymm8)
vblendvps ymm1, ymm1, ymm8, ymm9
vmulps ymm10, ymm6, ymm5
vblendvps ymm6, ymm6, ymm10, ymm9

vmovups [rcx + spheres.y + rdx * 4 - 32], ymm1
vmovups [rcx + spheres.v + rdx * 4 - 32], ymm6

sub rdx, 8
cmp rdx, 0
jg .cycle

.cycle_end:
mov rsp, rbp
pop rbp
ret 0

