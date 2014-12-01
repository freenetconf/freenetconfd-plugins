CC=gcc
CFLAGS=-g -shared -fPIC --std=gnu11
BUILD_DIR=build
INSTALL_DIR=/opt/git/freenetconfd/plugins

all: sandwich black-book shopping-list house-lockdown

build_dir:
	@if [ ! -d "${BUILD_DIR}" ]; then mkdir -p "${BUILD_DIR}"; fi

sandwich: build_dir sandwich.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/sandwich.so sandwich.c

black-book: build_dir black-book.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/black-book.so black-book.c

shopping-list: build_dir shopping-list.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/shopping-list.so shopping-list.c

house-lockdown: build_dir house-lockdown.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/house-lockdown.so house-lockdown.c

install:
	@cp $(BUILD_DIR)/* $(INSTALL_DIR)

clean:
	@rm -rf build

