RM = rm -f

uri: uri.tests.cpp ../src/Uri.cpp
	clang++ -g -fsanitize=address -o $@ $^
	./$@