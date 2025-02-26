CC = gcc

SRCS_ALL = main.c control/game_app.c engine/engine.c engine/render/quadmesh.c \
       utils/errors.c engine/text/text.c ../nanovg/src/nanovg.c \
       engine/buffers/index_buffer.c engine/buffers/vertex_buffer.c \
       engine/buffers/vertex_array.c engine/buffers/vertex_buffer_layout.c \
       engine/render/uniform_map.c engine/render/shader.c \
       engine/render/texture.c \
			 engine/objects/ink/ink.c engine/objects/tank/tank.c \
			 engine/physics/aabb.c

SRCS = $(shell find . -type f -name "*.c") \
			 ../nanovg/src/nanovg.c

LIBS = -lglfw -lGL -lGLEW -lm -ldl

TARGET = main

all:
	$(CC) -o $(TARGET) $(SRCS) $(LIBS) 

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean run
