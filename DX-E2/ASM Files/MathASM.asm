.code
	ClampASM proc
	xorps xmm2, xmm2
	maxss xmm0, xmm2
	minss xmm0, xmm1
	ret
	ClampASM endp
END