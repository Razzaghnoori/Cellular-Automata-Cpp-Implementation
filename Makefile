build: ca.cpp
	g++ -O3 -o ca -std=c++17 -I ${PWD} -pthread -w ca.cpp

run:
	./run.sh output.csv

clean:
	rm *.o ca