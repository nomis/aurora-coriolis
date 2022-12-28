import cbor2
import sys
import yaml

with open(sys.argv[1], "rb") as f_in:
	with open(sys.argv[2], "wb") as f_out:
		cbor2.dump(yaml.safe_load(f_in), f_out)
