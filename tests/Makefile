RM = rm -f

uri: uri.tests.cpp ../src/Uri.cpp
	clang++ -g -fsanitize=address -o $@ $^
	./$@

request: httprequest.tests.cpp ../src/HttpRequest.cpp ../src/Uri.cpp
	clang++ -g -fsanitize=address -o $@ $^
	./$@