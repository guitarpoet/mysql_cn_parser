#include "common.h"
#include "mysql_cn_parser.h"


int main() {
	reader = new INIReader("test/test.ini");
	Logger logger;
	if(reader->ParseError() < 0) {
        std::cout << "Can't load 'test.ini'\n";
		return 1;
	}
	logger.log("Hello %s", "Jack");
	std::cout << reader->Get("user", "email", "default_user") << std::endl;
	delete reader;
}
