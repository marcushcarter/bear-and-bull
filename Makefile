# === Configuration ===
CXX := g++
CXXFLAGS := -g -mwindows -Iinclude
LDFLAGS := -Llib -lmingw32 -lSDL3 -lSDL3_image -lSDL3_ttf
SRCS := src/main.c include/glad/glad.c resources/resource.res
OUT := main.exe

# === Targets ===

compile: build run

build: $(SRCS)
	windres resources/resource.rc -O coff -o resources/resource.res
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUT) $(LDFLAGS)

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile