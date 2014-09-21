#include "mysql_cn_parser.h"

int main() {
	std::cout << "Hello" << std::endl;
	INIReader reader("test/test.ini");

	if(reader.ParseError() < 0) {
        std::cout << "Can't load 'test.ini'\n";
		return 1;
	}

	std::cout << reader.Get("user", "email", "default_user") << std::endl;
}
