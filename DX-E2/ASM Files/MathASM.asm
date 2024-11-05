.code

;Clamps between 0 (xmm2)and a max value (xmm1)
ClampASM proc
	xorps xmm2, xmm2
	maxss xmm0, xmm2
	minss xmm0, xmm1
	ret
ClampASM endp

;Max value between (4) 32bit integers returned into eax.
MaxBetween4IntsASM proc
	cmp ecx, edx;
	cmovge edx, ecx;
	cmp edx, r8d;
	cmovl edx, r8d;
	cmp edx, r9d;
	cmovl edx, r9d;
	mov eax, edx;
	ret
MaxBetween4IntsASM endp

;Min value between (4) 32bit integers returned into eax.
MinBetween4IntsASM proc
	cmp ecx, edx;
	cmovle edx, ecx;
	cmp edx, r8d;
	cmovg edx, r8d;
	cmp edx, r9d;
	cmovg edx, r9d;
	mov eax, edx;
	ret
MinBetween4IntsASM endp



END

