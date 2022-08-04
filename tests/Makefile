RM = rm -f

uri: uri.tests.cpp ../src/Uri.cpp
	clang++ -g -fsanitize=address -o $@ $^

request: httprequest.tests.cpp ../src/HttpRequest.cpp ../src/Uri.cpp
	clang++ -g -fsanitize=address -o $@ $^

config: config.tests.cpp ../src/Config.cpp ../src/ServerConfig.cpp ../src/Route.cpp ../src/Uri.cpp ../src/File.cpp ../src/HttpRequest.cpp
	clang++ -g -fsanitize=address -o $@ $^
