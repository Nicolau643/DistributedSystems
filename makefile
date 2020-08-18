
SRC = $(wildcard source/*.c)
OBJ = $(patsubst source/%.c, object/%.o, $(SRC))

CC = gcc
LIBS = -lrt -lpthread -lzookeeper_mt
CFLAGS = -g -Wall -I ./include -I /usr/local/include 

all: $(OBJ) ## compile client, server and lib
	ld -r object/serialization.o object/read_write_all.o object/client_stub.o object/network_client.o object/data.o object/entry.o -o lib/client-lib.o
	gcc lib/client-lib.o object/table_client.o object/sdmessage.pb-c.o object/message.o -I /usr/local/include -L /usr/local/lib -l protobuf-c -o binary/table-client $(LIBS)
	gcc object/serialization.o object/read_write_all.o object/network_server.o object/table_skel.o object/table_server.o object/sdmessage.pb-c.o object/message.o object/table.o object/list.o object/entry.o object/data.o -I /usr/local/include -L /usr/local/lib -l protobuf-c -o binary/table-server $(LIBS)

table_server: $(OBJ) 
	$(CC) object/serialization.o object/read_write_all.o object/network_server.o object/table_skel.o object/table_server.o object/sdmessage.pb-c.o object/message.o object/table.o object/list.o object/entry.o object/data.o -I /usr/local/include -L /usr/local/lib -l protobuf-c -o binary/server 

table_client: $(OBJ)
	$(CC) lib/client-lib.o object/table_client.o object/sdmessage.pb-c.o object/message.o -I /usr/local/include -L /usr/local/lib -l protobuf-c -o binary/client

client-lib.o: $(OBJ)
	ld -r object/serialization.o object/read_write_all.o object/client_stub.o object/network_client.o object/data.o object/entry.o -o lib/client-lib.o

test_data: $(OBJETOS_data) ## compile data test
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS_data)) -o binary/test_data 

test_entry: $(OBJETOS_entry) ## compile entry test
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS_entry)) -o binary/test_entry

test_list: $(OBJETOS_list) ## compile list test
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS_list)) -o binary/test_list

test_table: $(OBJETOS_table) ## compile table test
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS_table)) -o binary/test_table

test_serialization: $(OBJETOS_serialization) ## compile serialization test
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS_serialization)) -o binary/test_serialization

run_all: all ## compile and run all tests
	cd ./binary && ./test_data && ./test_entry && ./test_list && ./test_table && ./test_serialization

run_data: test_data ## compile and run data test
	binary/test_data

run_entry: test_entry ## compile and run entry test
	binary/test_entry

run_list: test_list ## compile and run list test
	binary/test_list

run_table: test_table ## compile and run table test
	binary/test_table

run_serialization: test_serialization ## compile and run serialization test
	binary/test_serialization

clean: ## clean binary and object files
	(cd ./object && rm *)
	(cd ./binary && rm *)
	(cd ./lib && rm *)

clean_tests: ## clean test files from source folder
	cd ./source && rm *test_*

object/%.o: source/%.c
	/usr/local/bin/protoc --proto_path=source --c_out=source sdmessage.proto
	mv source/sdmessage.pb-c.h include
	$(CC) $(CFLAGS) -c $< -o $@

help: ## this help
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {sub("\\\\n",sprintf("\n%22c"," "), $$2);printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
