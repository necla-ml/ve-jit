	#addu.l %s0,60000,%s0
	#addu.l %s0,128,%s0
	#addu.l %s0,127,%s0
	#addu.l %s0,64,%s0
	addu.l %s0,63,%s0	# ok
	addu.l %s0,-63,%s0	# ok
	addu.l %s0,-64,%s0	# ok
	#addu.l %s0,-65,%s0
	#addu.l %s0,%s0,3
	addu.l %s0,%s0,(62)0
