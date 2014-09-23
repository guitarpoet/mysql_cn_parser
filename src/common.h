#ifndef MYSQL_CN_PARSER_COMMON

#define MYSQL_CN_PARSER_COMMON

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <stdarg.h>  // for va_start, etc
#include "INIReader.h"

#define DEFAULT_CONFIG_FILE "/etc/mysql_cn_parser.ini"

INIReader* reader;

inline bool file_exists (const std::string& name) {
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}

class Logger {
	private:
		bool enabled;
	public:
		Logger() {
			if(reader->Get("logger", "enabled", "0") == "1") {
				this->enabled = true;
			}
			else {
				this->enabled = false;
			}
		}
		void log(const std::string fmt, const std::string level = "INFO", ...) {
			if(!this->enabled)
				return;

			time_t rawtime;
			struct tm * timeinfo;
			char time_buffer [80];

			time (&rawtime);
			timeinfo = localtime (&rawtime);
			strftime (time_buffer, 80, "[%a %b %d %H:%M:%S %Y] ",timeinfo);

			int size = ((int)fmt.size()) * 2 + 50;   // use a rubric appropriate for your code
			std::string str;
			va_list ap;
			while (1) {     // maximum 2 passes on a POSIX system...
				str.resize(size);
				va_start(ap, level);
				int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
				va_end(ap);
				if (n > -1 && n < size) {  // everything worked
					str.resize(n);
					break;
				}
				if (n > -1)  // needed size returned
					size = n + 1;   // for null char
				else
					size *= 2;      // guess at a larger size (o/s specific)
			}
			if(reader->Get("logger", "console", "0") == "1") {
				std::cout << time_buffer << "[" << level << "] " << str << std::endl;
			}
			std::ofstream file;
			file.open(reader->Get("logger", "file", "/tmp/mysql_cn_parser.log"),  std::ios::out | std::ios::binary | std::ios::app);
			file << time_buffer << "[" << level << "] " << str << std::endl;
			file.close();
		}
};

#endif
