flatten: flatten.cc
	g++ -DFLATLIB_TEST -std=c++20 -Wall -Wextra -o $@ $< -ggdb
