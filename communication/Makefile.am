# compile protocol buffers for C
%.pb.cc %.pb.h: %.proto
	$(PROTOC) -I=$(dir $^) --cpp_out=$(dir $^) $^

# common static C libraries
noinst_LIBRARIES = libcommunication.a 
libcommunication_a_SOURCES = disambiguation_request.pb.cc connection.cc network.cc

clean-local:
	rm -f disambiguation_request.pb.cc
	rm -f disambiguation_request.pb.h
