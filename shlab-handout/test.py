import os

def rempid(s):
	while True:
		p = s.find('(', 0)
		if p == -1:
			break
		q = s.find(')', p)
		if q == -1:
			break
		s = s[:p] + s[q + 1:]
	return s

for i in range(1, 17):
	os.system('make test%02d 1>test.tmp' % i)
	os.system('make rtest%02d 1>rtest.tmp' % i)

	f = open('test.tmp')
	tr = f.read()
	f.close()

	f = open('rtest.tmp')
	rtr = f.read()
	f.close()

	tr = rempid(tr)
	rtr = rempid(rtr)

	tr = tr.strip()
	rtr = rtr.strip().replace('tshref', 'tsh')
	if tr != rtr:
		print tr
		print rtr
	else:
		print i