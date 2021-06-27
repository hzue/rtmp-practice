WORKDIR=.
BIN=$(WORKDIR)/bin/
LIB=$(WORKDIR)/lib/
INCLUDE=$(WORKDIR)/include/

.PHONY: bin

build:
	mkdir -p $(BIN)
	g++ -Wall -std=c++11 -o bin/main src/main.cpp -I $(INCLUDE) -L $(LIB) -lavformat.58 -lavcodec.58

bin:
	@DYLD_LIBRARY_PATH=$(LIB) ./bin/main

clean:
	$(RM) -rf bin/main

########################
## RTMP Server:
## crtmpserver, nginx_rtmp
########################
build-%:
	@cd dockerfile/; \
		docker build -f Dockerfile.$* -t terrychu/$*:latest .

run-%:
	docker run -it --rm \
		-p 8080:8080 \
		-p 1935:1935 \
		terrychu/$*:latest  # 8080 is only for nginx
