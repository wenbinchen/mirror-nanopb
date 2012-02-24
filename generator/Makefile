ifndef PB_PATH
PBPATHOPT=-I/usr/include -I/usr/local/include
else
PBPATHOPT=-I$(PB_PATH)
endif

nanopb_pb2.py: nanopb.proto
	protoc --python_out=. $(PBPATHOPT) -I . nanopb.proto
