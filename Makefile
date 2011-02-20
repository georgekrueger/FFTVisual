GLEW_INCLUDE = ../glew/include
GLEW_LIB = ../glew/lib

fft_visual: fft_visual.o
	g++ -o fft_visual $^ -framework GLUT -framework OpenGL -L$(GLEW_LIB) -lGLEW

.c.o:
	g++ -c -o $@ $< -I$(GLEW_INCLUDE) -g

clean:
	rm -f fft_visual fft_visual.o

