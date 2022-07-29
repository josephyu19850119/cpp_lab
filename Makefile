proto_src : addressbook.proto
	protoc --cpp_out=. addressbook.proto
reader : reader.cpp addressbook.pb.h addressbook.pb.cc
	g++ -std=c++11 -g reader.cpp addressbook.pb.cc -o reader -lprotobuf
writer : writer.cpp addressbook.pb.h addressbook.pb.cc
	g++ -std=c++11 -g writer.cpp addressbook.pb.cc -o writer -lprotobuf
clean_src :
	rm addressbook.pb.h addressbook.pb.cc
clean_bin :
	rm writer reader