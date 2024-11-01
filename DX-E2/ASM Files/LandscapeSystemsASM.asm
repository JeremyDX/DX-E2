.data
	upper_bound REAL4 1023.0f

.code
	ClampASM proc
	xorps xmm1, xmm1
	maxss xmm0, xmm1
	minss xmm0, upper_bound
	ret
	ClampASM endp
END