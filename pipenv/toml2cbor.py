import cbor2
import sys
import tomli

with open(sys.argv[1], "rb") as f_in:
	with open(sys.argv[2], "wb") as f_out:
		cbor2.dump(tomli.load(f_in), f_out)
