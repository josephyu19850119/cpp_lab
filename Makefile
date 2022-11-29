proto_src : underlay_message.proto
	protoc --cpp_out=. underlay_message.proto
player : msg_player.cpp underlay_message.pb.h underlay_message.pb.cc
	g++ -std=c++11 -g msg_player.cpp underlay_message.pb.cc -o msg_player -lprotobuf
clean_src :
	rm underlay_message.pb.h underlay_message.pb.cc
clean_bin :
	rm msg_player