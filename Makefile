CXX=g++

INCPATH=-I. -IItem -INetWork -I$(PROTOBUF_DIR)/include -I$(BOOST_ROOT)
CXXFLAGS += $(OPT) -pipe -Wno-unused-local-typedefs -Wno-unused-but-set-variable -Wno-literal-suffix -Wall -std=c++11 -ggdb -fPIC -D_GNU_SOURCE -D__STDC_LIMIT_MACROS $(INCPATH)

LIBRARY=$(PROTOBUF_DIR)/lib/libprotobuf.a -L$(BOOST_ROOT)/stage/lib/
LDFLAGS = -lboost_system -lboost_thread

PROTO_SRC=P_Assert.proto P_Property.proto P_Protocol.proto P_Server.proto
PROTO_OBJ=$(patsubst %.proto,%.pb.o,$(PROTO_SRC))
PROTO_OPTIONS=--proto_path=. --proto_path=$(PROTOBUF_DIR)/include

BASE_OBJ=WorldSession.o Protocol.o Entity.o Player.o World.o Asset.o Scene.o
SUB_OBJ=Item/*.o

BIN=GameServer

all: $(BIN)

clean:
	@rm -f $(BIN) $(SUB_OBJ) *.o *.pb.*

rebuild: clean all

GameServer: $(PROTO_OBJ) $(BASE_OBJ) $(SUB_OBJ) Main.o
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

%.pb.cc: %.proto
	$(PROTOBUF_DIR)/bin/protoc $(PROTO_OPTIONS) --cpp_out=. $<

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
