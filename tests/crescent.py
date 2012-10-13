from struct import *
import sys

types = {
	1: "<BBHdddfffffHH",
	95: "<HxxL 10L 10L 10L",
	76: "<dH8x 81L 15L 12L",
	96: "<xxHd " + ''.join(("BBBB" for i in range(12))) + "12L 12d 12d",
	98: "<" + ''.join(("HBBBBbB" for i in range(8))) + "BBxx",
}

def read_or_die(length):
	ret = sys.stdin.read(length)
	if len(ret) == 0:
		sys.exit(0)
	return ret

def match(expected):
	return read_or_die(len(expected)) == expected

def get_chunks(fmt):
	return unpack(fmt, read_or_die(calcsize(fmt)))

def compute_checksum(body):
	return sum((ord(c) for c in body)) & 0xffff

while True:
	if not match("$BIN"):
		sys.stdin.readline()
		continue
	blockid, datalength = get_chunks("<HH")
	body = read_or_die(datalength)
	checksum, = get_chunks("<H")
	if checksum != compute_checksum(body) or not match("\r\n"):
		print "bad format"
		sys.stdin.readline()
		continue
	if blockid in types:
		body = unpack(types[blockid], body)
	else:
		body = "(unparsed)"
	print blockid, body
