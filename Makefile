CC = gcc

SRC = main.c
HEADER = timProcess.h tmdProcess.h vdfProcess.h datProcess.h model.h common.h
TARGET = tmdd
STATIC_LIB =
CFLAGS = -O2 -Wall 

UNAME_S := $(shell uname -s)

# sigh

ifeq ($(OS),Windows_NT)
    CFLAGS += -Iraylib-mingw/include -Lraylib-mingw/lib -lopengl32 -lgdi32 -lwinmm
    RM = del /Q
    STATIC_LIB += raylib-mingw/lib/libraylib.a
else
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -Iraylib-macos/include -Lraylib-macos/lib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreAudio -framework CoreVideo -framework AudioToolbox
        RM = rm -f
        STATIC_LIB += raylib-macos/lib/libraylib.a
    else
        CFLAGS += -Iraylib-linux/include -Lraylib-linux/lib -lGL -lm -lpthread -ldl -lrt -lX11
        RM = rm -f
        STATIC_LIB += raylib-linux/lib/libraylib.a
    endif
endif

all: $(TARGET)

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(STATIC_LIB)

clean:
	$(RM) $(TARGET)
