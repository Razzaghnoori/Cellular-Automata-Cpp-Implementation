build: ca.cpp
	g++ -O3 -std=c++17 -I ${HOME}/fastflow -pthread -w -o ca ca.cpp
seq: sequential.cpp
	g++ -O3 -std=c++17 -I ${HOME}/fastflow -pthread -w -o sequential sequential.cpp
par: parallel.cpp
	g++ -O3 -std=c++17 -I ${HOME}/fastflow -pthread -w -o parallel parallel.cpp
ff: fastflow.cpp
	g++ -O3 -std=c++17 -I ${HOME}/fastflow -pthread -w -o fastflow fastflow.cpp
clean:
	rm *.o ca