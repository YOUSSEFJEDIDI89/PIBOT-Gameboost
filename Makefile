# بناء بديل بدون CMake:  make && make install
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
PREFIX   ?= $(HOME)/../usr

SRC := $(wildcard src/*.cpp)

pibot: $(SRC) src/json.hpp src/util.hpp src/http.hpp src/priv.hpp src/boost.hpp src/game.hpp src/bot.hpp
	$(CXX) $(CXXFLAGS) $(SRC) -o pibot

install: pibot
	install -m 755 pibot $(PREFIX)/bin/pibot

clean:
	rm -f pibot

.PHONY: install clean
