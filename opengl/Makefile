# Compiler and flags
CC = gcc

# Source files
SRCS = main.c control/game_app.c engine/engine.c engine/render/quadmesh.c \
       utils/errors.c engine/text/text.c ../nanovg/src/nanovg.c \
       engine/buffers/index_buffer.c engine/buffers/vertex_buffer.c \
       engine/buffers/vertex_array.c engine/buffers/vertex_buffer_layout.c \
       engine/render/uniform_map.c engine/render/shader.c \
       engine/render/texture.c \
			 engine/objects/ink/ink.c engine/objects/tank/tank.c engine/physics/aabb.c

# Libraries to link with
LIBS = -lglfw -lGL -lGLEW -lm -ldl

# Output binary name
TARGET = main

# Default rule to build the project
all:
	$(CC) -o $(TARGET) $(SRCS) $(LIBS) 

# Rule to run the program
run: all
	./$(TARGET)

# Clean rule to remove the binary
clean:
	rm -f $(TARGET)

.PHONY: all clean run

