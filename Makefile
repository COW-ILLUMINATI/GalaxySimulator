
build_program:
	g++ src/Simulator.cpp -lSDL2 -fopenmp -o simulator.out
	g++ src/Creator.cpp -lSDL2 -fopenmp -o creator.out
