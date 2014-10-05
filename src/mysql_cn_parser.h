#ifndef MYSQL_CN_PARSER

#define MYSQL_CN_PARSER
#define TOKEN_TYPE const char*
#define TOKEN_TYPE_TOKEN "TOKEN"
#define TOKEN_TYPE_LINE_BREAK "LINE_BREAK"
#define TOKEN_TYPE_STOP_WORD "STOP_WORD"

//-----------------------------------------------------------------------------------------
//
// The operator types
//
//-----------------------------------------------------------------------------------------
#define OPERATOR_TYPE_PLUS "+"
#define OPERATOR_TYPE_MINUS "-"
#define OPERATOR_TYPE_GT ">"
#define OPERATOR_TYPE_LT "<"
#define OPERATOR_TYPE_LP "("
#define OPERATOR_TYPE_RP ")"
#define OPERATOR_TYPE_TILDE "~"
#define OPERATOR_TYPE_ASTERISK "*"

#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <mmseg/Segmenter.h>
#include <mmseg/SegmenterManager.h>
#include <mysql/plugin.h>
#include "common.h"

int mysql_cn_parser_parse(MYSQL_FTPARSER_PARAM *param);
int mysql_cn_parser_init(MYSQL_FTPARSER_PARAM *param __attribute__((unused)));
int mysql_cn_parser_deinit(MYSQL_FTPARSER_PARAM *param __attribute__((unused)));

struct st_mysql_ftparser mysql_cn_parser_descriptor =
{
  MYSQL_FTPARSER_INTERFACE_VERSION,     /* interface version      */
  mysql_cn_parser_parse,                /* parsing function       */
  (mysql_cn_parser_init),               /* parser init function   */
  (mysql_cn_parser_deinit)              /* parser deinit function */
};

char current_status[255];
long number_of_calls = 0;

struct st_mysql_show_var mysql_cn_status[] =
{
  {"mysql_cn_status", (char *)&current_status, SHOW_CHAR},
  {"mysql_cn_called", (char *)&number_of_calls, SHOW_LONG}
};

mysql_declare_plugin(mysql_cn_parser)
{
  MYSQL_FTPARSER_PLUGIN,                    /* type                            */
  &mysql_cn_parser_descriptor,              /* descriptor                      */
  "mysql_cn_parser",                        /* name                            */
  "Jack <guitarpoet@gmail.com>",            /* author                          */
  "Simple Chinese MMSEG Parser",            /* description                     */
  PLUGIN_LICENSE_GPL,                       /* plugin license                  */
  (int (*)(void*))mysql_cn_parser_init,     /* init function (when loaded)     */
  (int (*)(void*))mysql_cn_parser_deinit,   /* deinit function (when unloaded) */
  0x0001,                                   /* version                         */
  NULL,                          /* status variables                */
  NULL,                                     /* system variables                */
  NULL
}
mysql_declare_plugin_end;

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
			std::ifstream _file(stopWordsFile.c_str());
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
			logger->log("Stop words file %s readed", "INFO", stopWordsFile.c_str());
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

Parser* parser;

#endif
