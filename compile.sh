if [[ "$1" == "graph" ]]
then
	clang++ src/graph.cpp -c -o src/graph.o -std=c++17 -O3
fi

if [[ "$1" == "make" ]]
then
	clang++ src/make_graph.cpp -o make -std=c++17 -O3 /usr/local/lib/libbz2.a /usr/local/lib/libexpat.a /usr/local/lib/libz.a src/graph.o
fi

if [[ "$1" == "results" ]]
then
	clang++ src/results.cpp -o results -std=c++17 -O3 src/graph.o
fi
