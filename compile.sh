if [[ "$1" == "make" ]]
then
	clang++ src/make_graph.cpp -o make -std=c++17 -O3 /usr/local/lib/libbz2.a /usr/local/lib/libexpat.a /usr/local/lib/libz.a
fi

if [[ "$1" == "run" ]]
then
	clang++ src/run.cpp -o run -std=c++17 -O3
fi