# projekt.s
# Autor: Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz

.intel_syntax noprefix

.data
	# Maximální vzdálenost barev ve 24b RGB: sqrt((255-0)^2*3) = 441.67
	large_floats: .single 999999.0, 999999.0, 999999.0, 999999.0
	zeros: .quad 0, 0
	ones: .single 1.0, 1.0, 1.0, 1.0
	
	grayscale_consts: .single 0.114, 0.587, 0.299, 0.0, 0.114, 0.587, 0.299, 0.0
	max_consts: .single 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0, 255.0
	shuf_const: .quad 0xFFFFFFFFFFFFFFFFFF050403FF020100
	shuf_back_const: .quad 0xFFFFFFFFFFFFFFFFFFFFFFFFFF040200
.text

# Pro ladění:
# .extern printCentroid

# void algorithm_kmeans(RCX uchar *imageData, RDX centroid *centroidMemory, R8 long centroidCount, R9 int *results, [rbp+0x30] long pixelCount)
.global algorithm_kmeans
algorithm_kmeans:
push rbp
mov rbp, rsp
push r15
push r14
push r13
push rsi
	# Do R15 si uložím pixelCount
	mov r15, [rbp+0x30] # Nechť Windows x64 ABI shoří v pekle
	# Do RSI si uložím adresu dat (RCX)
	mov rsi, rcx
	# Do XMM15 si uložíme konstanty s "maximální vzdáleností"
	movups xmm15, [rip + large_floats]
	# Do XMM12 si uložíme nuly
	movups xmm12, [rip + zeros]
	# Do XMM9 si uložíme jedničky
	movups xmm9, [rip + ones]
.main_cycle:
	# ---- první cyklus - prochází přes pixely a počítá vzdálenosti ----
	# RCX = p = čítač na pixely
	xor rcx, rcx # p = 0
	.pixel_cycle:
		# v xmm14 si budeme pro aktuální 4 pixely držet minDistance, začínáme na maximech
		movaps xmm14, xmm15
		# v xmm13 si budeme pro aktuální 4 pixely držet minCentroidIndex, začínáme na nule
		movaps xmm13, xmm12

		imul rax, rcx, 3

		# Načteme složky čtyř pixelů do 4PS vektorů XMM3 (R), XMM2 (G) a XMM1 (B).
		# Pixely jsou ale uloženy za sebou, takže to musíme přeshufflovat.
		# Načteme čtyři sekvence BAJTŮ:         (MSB) R3 G3 B3 R2 |        G2 B2 R1 G1 |        B1 R0 G0 B0 (LSB)        (96 bitů)
		# A potřebujeme z toho udělat FLOATY:  xmm3<- R3 R2 R1 R0 | xmm2<- G3 G2 G1 G0 | xmm1<- B3 B2 B1 B0 (LSvecfield) (3*128 bitů)

		pmovzxbd xmm0, [rsi + rax] # Načtu 4 bajty z paměti a udělám z toho čtyři 32b inty v xmm15
		cvtdq2ps xmm0, xmm0 # A z nich čtyři floaty (4PS)
		
		# A to samé pro další dva kusy:
		# (G1–G2)
		pmovzxbd xmm1, [rsi + rax + 4]
		cvtdq2ps xmm1, xmm1
		# (R2–R3)
		pmovzxbd xmm2, [rsi + rax + 8]
		cvtdq2ps xmm2, xmm2

		# Skvěle, mám uložené tři bloky floatů B0–B1 v xmm0, G1–G2 v xmm1 a R2–R3 v xmm2
		# Nyní s tím zamíchám, abych to měl pěkně za sebou
		
		# krok 1: R
		vshufps xmm3, xmm0, xmm1, 0b01011010 # xmm3 <- R1 R1 R0 R0
		vshufps xmm3, xmm3, xmm2, 0b11001000 # xmm3 <- R3 R2 R1 R0

		# krok 2: G
		vshufps xmm4, xmm0, xmm2, 0b10100101 # xmm4 <- G3 G3 G0 G0 (2,2,1,1)
		vshufps xmm4, xmm1, xmm4, 0b10001100 # xmm4 <- G3 G0 G2 G1 (2,0,3,0)
		shufps xmm4, xmm4, 0b11010010        # xmm4 <- G3 G2 G1 G0 (3,1,0,2)

		# krok 3: B
		vshufps xmm5, xmm1, xmm2, 0b01011010 # xmm5 <- B3 B3 B2 B2
		vshufps xmm5, xmm0, xmm5, 0b10001100 # xmm5 <- B3 B2 B1 B0

		# checkpoint:
		# využíváme XMM3, XMM4, XMM5 (pro složky 4 pixelů)
		#           XMM12 (pro konstantní nuly), XMM15 (pro konstantu maxDistance)
		#			XMM13 (pro minCentroidIndex 4 pixelů), XMM14 (pro minDistance 4 pixelů)
		#           RSI (*imageData), RCX (čítač p), RDX (*centroidMem), R8 (centroidCount),
		#  			R9 (*results), R15 (imageDataSize)

		# R14 = čítač na centroidy
		xor r14, r14
		.centroids_cycle_1:
			# vynásobíme si index hodnotou 32
			imul rax, r14, 32
			# xmm0 <- 0 ci_B ci_G ci_R
			movaps xmm0, [rdx + rax]
			# xmm1 <- ci_G ci_G ci_G ci_G
			vshufps xmm1, xmm0, xmm0, 0b01010101
			# xmm2 <- ci_B ci_B ci_B ci_B
			vshufps xmm2, xmm0, xmm0, 0b10101010
			# xmm0 <- ci_R ci_R ci_R ci_R (!)
			shufps xmm0, xmm0, 0

			# Nemůžeme upravovat načtené hodnoty 4 pixelů, protože je budeme potřebovat pro další centroid
			# Takže budeme počítat nad XMM6–8
			# (pR - cR) atd.
			vsubps xmm6, xmm0, xmm3
			vsubps xmm7, xmm1, xmm4
			vsubps xmm8, xmm2, xmm5
			# (pR - cR) * (pR - cR)
			mulps xmm6, xmm6
			mulps xmm7, xmm7
			mulps xmm8, xmm8
			# sečtu to všechno do xmm6
			addps xmm6, xmm7
			addps xmm6, xmm8
			# a odmocním
			# xmm6 <- distance_p3 distance_p2 distance_p1 distance_p0
			sqrtps xmm6, xmm6
			# xmm7 <- maska porovnání s aktuálními maximy
			vcmpps xmm7, xmm6, xmm14, 1 # 1 = less than (distance_ < minDistance_)
			# podle masky nastavím minDistance_
			vblendvps xmm14, xmm14, xmm6, xmm7
			# a taky minCentroidIndex -> musím si akutální index načíst a rozdistr. do xmm11
			movq xmm11, r14
			shufps xmm11, xmm11, 0 # "prasácky" si vezmeme jen 32 bitů, i když je counter 64b - tolik centroidů nikdy mít nebudeme
			vblendvps xmm13, xmm13, xmm11, xmm7
		inc r14
		cmp r14, r8
		jl .centroids_cycle_1

		# A teď musíme podle indexů v xmm13 nastavit položky v paměti [rdx] a přičíst k nim ty barvy :HAhaa:
		# To asi nepůjde moc jinak než ručně
		# Do R14 si postupně budu ukládat jednotlivé minCentroidIndex pro každý ze 4 pixelů
		xor r14, r14
		
		# Pro bod 0:
		pextrd r14d, xmm13, 0
		imul r14d, r14d, 32
		add r14d, 16 # [rdx + r14] ukazuje na část struktury centroid, kde jsou uložené nové hodnoty

		vinsertps xmm11, xmm3, xmm4, 0b00011100
		vinsertps xmm10, xmm5, xmm9, 0b00011100
		shufps xmm10, xmm10, 0b01001111
		addps xmm10, xmm11 # v xmm10 mám opět složený bod 0: xmm10 <- 1 B0 G0 R0

		movaps xmm11, [rdx + r14]
		addps xmm11, xmm10
		movaps [rdx + r14], xmm11

		# Pro bod 1:
		pextrd r14d, xmm13, 1
		imul r14d, r14d, 32
		add r14d, 16

		vinsertps xmm11, xmm4, xmm3, 0b01001100
		vinsertps xmm10, xmm5, xmm9, 0b00001100
		shufps xmm10, xmm10, 0b00011111
		addps xmm10, xmm11 # xmm10 <- 1 B1 G1 R1

		movaps xmm11, [rdx + r14]
		addps xmm11, xmm10
		movaps [rdx + r14], xmm11

		# Pro bod 2:
		pextrd r14d, xmm13, 2
		imul r14d, r14d, 32
		add r14d, 16

		vinsertps xmm11, xmm3, xmm4, 0b10011001
		vinsertps xmm10, xmm5, xmm9, 0b00011001
		vshufps xmm10, xmm11, xmm10, 0b01100110 # xmm10 <- 1 B2 G2 R2

		movaps xmm11, [rdx + r14]
		addps xmm11, xmm10
		movaps [rdx + r14], xmm11
		
		# Pro bod 3:
		pextrd r14d, xmm13, 3
		imul r14d, r14d, 32
		add r14d, 16

		vinsertps xmm11, xmm3, xmm4, 0b11100011
		vinsertps xmm10, xmm5, xmm9, 0b00010101
		vshufps xmm10, xmm11, xmm10, 0b01111011 # xmm10 <- 1 B3 G3 R3

		movaps xmm11, [rdx + r14]
		addps xmm11, xmm10
		movaps [rdx + r14], xmm11

		# Nastavíme příslušnou část pole results
		movaps [r9 + rcx * 4], xmm13

		# Ladicí kód pro výpis obsahů centroidů:
		/*
		push rax
		push rcx
		push rdx
		push r8
		push r9
		push r14
		push r15
		call printCentroids
		pop r15
		pop r14
		pop r9
		pop r8
		pop rdx
		pop rcx
		pop rax
	*/

	add rcx, 4  # p += 4
	cmp rcx, r15 # if p < pixelCount, goto .pixel_cycle
	jl .pixel_cycle

	# ---- druhý cyklus - prochází přes centroidy a zjišťuje, jestli došlo ke změně ----
	# R14 = hadChange - flag o změně
	xor r14, r14
	# RCX = ci
	xor rcx, rcx
	.centroids_cycle_2:
		# vynásobíme si index hodnotou 32
		imul rax, rcx, 32

		# xmm0 <- 0 ci_B ci_G ci_R
		movaps xmm0, [rdx + rax]
		# xmm1 <- ci_newPointsCount ci_newB ci_newG ci_newR
		movaps xmm1, [rdx + rax + 16]
		# xmm2 <- ci_nPC ci_nPC ci_nPC ci_nPC
		vshufps xmm2, xmm1, xmm1, 0b11111111

		# Pokud ci_nPC == 0, skočme, bude to rychlejší než to všechno zbytečně počítat
		comiss xmm2, xmm12
		je .centroids_cycle_2_end

		# xmm3 <- xmm1 / xmm2
		vdivps xmm3, xmm1, xmm2

		# uschovejme si v xmm4 původní načtené hodnoty a v xmm1 nové hodnoty (ve floatech)
		movaps xmm4, xmm0
		movaps xmm1, xmm3

		# převedeme na celá čísla, aby bylo porovnávání radostnější
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm3, xmm3
		
		# xmm0 <- maska porovnání - pokud se rovnají, bude to plné jedniček
		# čtvrtou složku vektoru ignorujeme
		vpcmpeqd xmm0, xmm0, xmm3 
		pextrq r13, xmm0, 0
		not r13
		or r14, r13
		pextrd r13d, xmm0, 2
		not r13d
		or r14d, r13d
		# pokud tam došlo k nějaké změně, v r14 bude číslo rozdílné od 0

		# vynulujeme v xmm1 ci_nPC a uložíme do paměti jako nové hodnoty centroidu
		vblendps xmm1, xmm1, xmm12, 0b00001000

		movaps [rdx + rax], xmm1
		# vynulujeme v paměti newR/G/B/PC
		movaps [rdx + rax + 16], xmm12

	.centroids_cycle_2_end:
	inc rcx
	cmp rcx, r8
	jl .centroids_cycle_2

	# pokud došlo ke změně, skočme na začátek
	cmp r14, 0
	jne .main_cycle

pop rsi
pop r13
pop r14
pop r15
mov rsp, rbp
pop rbp
ret 0


# void algorithm_imgproc(RCX uchar *imageData, RDX uchar *outputData, R8 int *results, R9 int centroidIndex, [rbp+0x30] long pixelCount)
.global algorithm_imgproc
algorithm_imgproc:
push rbp
mov rbp, rsp
push r15
push r10
push rbx
push rsi
push rdi

	mov r15, [rbp+0x30]
	mov rdi, rdx
	mov rsi, rcx
	
	# Budeme pracovat s 256b YMM registry.
	vmovups ymm15, [rip+grayscale_consts]
	vmovups ymm14, [rip+max_consts]
	movups xmm13, [rip+shuf_const]
	movups xmm12, [rip+shuf_back_const]

	# rcx = p = čítač pixelů
	xor rcx, rcx
	.pixel_cycle_imgproc:
		# načteme masku - do prvních třech chceme položek results[p], do dalších třech položek results[p + 1]

		# první načteme 64 b – dva inty
		movq xmm1, [r8+rcx*4]
		vshufps xmm2, xmm1, xmm1, 0b11000000 # xmm2 <- 0  res_p1 res_p1 res_p1
		vshufps xmm3, xmm1, xmm1, 0b11010101 # xmm3 <- 0  res_p2 res_p2 res_p2
		vinsertf128 ymm1, ymm2, xmm3, 1      # ymm1 <- 0  res_p2 res_p2 res_p2 | 0  res_p1 res_p1 res_p1
		# načtu a rozdistribuji R9 do ymm2
		movd xmm2, r9
		vbroadcastss ymm2, xmm2
		# porovnám, maska v ymm1
		vpcmpeqd ymm1, ymm1, ymm2

		imul rax, rcx, 3

		movq xmm2, [rsi+rax]
		pshufb xmm2, xmm13
		vpmovzxbd ymm2, xmm2 # AVX2 :(
		vmovaps ymm3, ymm2
		# ymm2, ymm3 <- 0  r_2 g_2 b_2 | 0  r_1 g_1 b_1 (int32)

		# převedeme uvnitř registru na floaty
		vcvtdq2ps ymm2, ymm2 # ymm2 <- 0  r_2 g_2 b_2 | 0  r_1 g_1 b_1 (float32)

		# vynásobíme konstantami pro převod do grayscale
		vmulps ymm2, ymm2, ymm15

		# sečteme
		vhaddps ymm2, ymm2, ymm2
		vhaddps ymm2, ymm2, ymm2 # ymm2 <- gs2 gs2 gs2 gs2 | gs1 gs1 gs1 gs1

		# převedeme zpět na 32b inty
		vcvtps2dq ymm2, ymm2

		# teď musíme na vymaskovaná místa dostat původní hodnoty (v ymm3)
		vblendvps ymm2, ymm2, ymm3, ymm1

		# a teď zase horní půlku strčíme do xmm1 a převedeme to na bajty (unsigned saturace -> zastropuje na 255)
		vextractf128 xmm1, ymm2, 1
		packuswb xmm2, xmm2 # xmm2 (bytes, 63–0) <- 0 gs1 0 gs1 0 gs1 0 gs1
		packuswb xmm1, xmm1 # xmm1 <- same for gs0
		pshufb xmm2, xmm12
		pshufb xmm1, xmm12

		movd rbx, xmm1
		movd r10, xmm2
		and r10, 0xFFFFFF
		shl rbx, 24
		or r10, rbx

		movq xmm2, r10

		mov [rdi+rax], r10
	add rcx, 2
	cmp rcx, r15
	jl .pixel_cycle_imgproc

pop rdi
pop rsi
pop rbx
pop r10
pop r15
mov rsp, rbp
pop rbp
ret 0
