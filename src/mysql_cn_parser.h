#ifndef MYSQL_CN_PARSER

#define MYSQL_CN_PARSER
#define TOKEN_TYPE const char*
#define TOKEN_TYPE_TOKEN "TOKEN"
#define TOKEN_TYPE_LINE_BREAK "LINE_BREAK"

#include <iostream>
#include <mmseg/Segmenter.h>
#include <mmseg/SegmenterManager.h>
#include "common.h"

/**
 * The initialize function for initialize the parser using configurations.
 */
inline INIReader* read_config() {
	INIReader* reader = NULL;
	if(file_exists(DEFAULT_CONFIG_FILE)) {
		reader = new INIReader(DEFAULT_CONFIG_FILE);
	}
	else {
		if(file_exists("mysql_cn_parser.ini")) {
			reader = new INIReader("mysql_cn_parser.ini");
		}
	}
	if(!reader) {
		std::cerr << "Can't find the configuration file for mysql_cn_parser." << std::endl;
		return NULL;
	}
	if(reader->ParseError() < 0) {
        std::cerr << "Can't load configuration file." << std::endl;
	}
	return reader;
}

struct TokenPeek {
	char* tok;
	u2 len;
	u2 symlen;
	TOKEN_TYPE type;
};

typedef struct TokenPeek token_peek;

class Parser {
	private: 
		INIReader* reader;
		css::SegmenterManager* mgr;
		css::Segmenter* seg;
		std::vector<std::string>* stopwords;


	public:
		Logger* logger;
		bool inited;

	private:
		bool isStopWord(std::string item) {
			return std::binary_search(stopwords->begin(), stopwords->end(), item);
		}

		void readStopWords(std::string stopWordsFile) {
			stopwords = new std::vector<std::string>();
			std::ifstream _file(stopWordsFile);
			std::copy(std::istream_iterator<std::string>(_file),
				std::istream_iterator<std::string>(),
				std::back_inserter(*stopwords));
			std::sort(stopwords->begin(), stopwords->end());
		}
		bool filterToken(const char* token, u2 len, u2 symlen);

	public:
		Parser(INIReader* reader) : reader(reader), mgr(new css::SegmenterManager()),
			logger(new Logger()) {
			inited = false;
			logger->log("Initializing the segmenter...");
			std::string data_dir = reader->Get("parser", "data_dir", "/usr/local/etc");
			if(!file_exists(data_dir)) {
				logger->log("The data dir %s is not found", "ERROR", data_dir.c_str());
				return;
			}
			mgr->init(data_dir.c_str());
			seg = mgr->getSegmenter();
			logger->log("Segmenter initialized.");

			std::string stopWordsFile = reader->Get("parser", "stop_words", "stopwords.txt");
			logger->log("Reading stop words from file %s", "INFO", stopWordsFile.c_str());
			readStopWords(stopWordsFile);
			inited = true;
		}

		~Parser() {
			logger->log("Deinitializing parser...");

			if(stopwords)
				delete stopwords;
			if(logger)
				delete logger;
			if(mgr)
				delete mgr;
		}

		void set(const char* buffer, int length) {
			seg->setBuffer((u1*)buffer, length);
		}

		bool peek(token_peek &peek);
		TOKEN_TYPE next(char* result);
};

#endif
