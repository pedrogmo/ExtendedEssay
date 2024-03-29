if [[ "$1" == "graph" ]]
then
	clang++ src/graph.cpp -c -o src/graph.o -std=c++17 -O3
fi

if [[ "$1" == "make" ]]
then
	clang++ src/make_graph.cpp -o make -std=c++17 -O3 /usr/local/lib/libbz2.a /usr/local/lib/libexpat.a /usr/local/lib/libz.a src/graph.o
fi

if [[ "$1" == "run" ]]
then
	clang++ src/run.cpp -o run -std=c++17 -O3 src/graph.o
fi

if [[ "$1" == "factor" ]]
then
	clang++ src/factor.cpp -o factor -std=c++17 -O3 src/graph.o
fi

if [[ "$1" == "results" ]]
then
	clang++ src/results.cpp -o results -std=c++17 -O3 src/graph.o
fi

if [[ "$1" == "speed" ]]
then
	clang++ src/average_speed.cpp -o speed -std=c++17 -O3 /usr/local/lib/libbz2.a /usr/local/lib/libexpat.a /usr/local/lib/libz.a
fi