#include <stdlib.h>
#include <ctype.h>
#include "mysql_cn_parser.h"
#include <cassert>

using namespace __gnu_cxx;

void set_status(const char* status) {
    std::strcpy(current_status, status);
}

bool str_eql(const char* str1, const char* str2) {
	return strncmp(str1, str2, sizeof(str2) - 1) == 0;
}

int mysql_cn_parser_init(MYSQL_FTPARSER_PARAM *param __attribute__((unused))) {
    set_status("Initializing using dictionary...");
	reader = read_config();
	parser = new Parser(reader);
	parser->logger->log("Parser initialize complete");
    return 0;
}

int mysql_cn_parser_deinit(MYSQL_FTPARSER_PARAM *param __attribute__((unused))) {
	parser->logger->log("Destroying parser...");
	delete parser;
	delete reader;
    return 0;
}

int mysql_cn_parser_parse(MYSQL_FTPARSER_PARAM *param) {
    set_status("Tokenizing the string");
	parser->logger->log("Tokenizing the string");

	TOKEN_TYPE t;
	char tok[1024];
	parser->set(param->doc, param->length);
    while((t = parser->next(tok))) {
		if(t == TOKEN_TYPE_TOKEN) {
			switch(param->mode) {
				case MYSQL_FTPARSER_SIMPLE_MODE:
					break;
				case MYSQL_FTPARSER_WITH_STOPWORDS:
					break;
				case MYSQL_FTPARSER_FULL_BOOLEAN_INFO:
					break;
			}
			MYSQL_FTPARSER_BOOLEAN_INFO bool_info = {
				FT_TOKEN_WORD, // Token type
				0, // Yes No - Use no by default
				0, // Weight Adjust - No adjust by default
				0, // Weight Adjust Sign - No sign
				0, // Trunk
	//          pos, // Position
				' ', // Prev
				0 // Quote
			};
			int ret = param->mysql_add_word(param, tok, strlen(tok), &bool_info);
			std::cout << "Adding Token [" << tok << "] with length " << strlen(tok)
			   << " getting result " << ret << std::endl;
		}
    }


    return 0;
}
int main() {
	std::auto_ptr<INIReader> reader_ptr(read_config());
	reader = reader_ptr.get();

	if(!reader)
		return -1;

	Logger logger;

	logger.log("Reading the testing file.");
    std::istream *is = new std::ifstream("test/data.txt", std::ifstream::in | std::ifstream::binary);
    if(! *is)
        return -1;

    int length;
    is->seekg (0, std::ifstream::end);
    length = is->tellg();
    is->seekg (0, std::ifstream::beg);
    char* buffer = new char [length+1];
    is->read (buffer,length);
    buffer[length] = 0;
    delete is;

	std::auto_ptr<Parser> parser_ptr(new Parser(reader));
	Parser* parser = parser_ptr.get();
	parser->set(buffer, length);

	char result[1024];
	TOKEN_TYPE t;
	while((t = parser->next(result))) {
		if(t == TOKEN_TYPE_TOKEN)
			std::cout << result << "/x ";
		else
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

bool Parser::filterToken(const char* token, u2 len, u2 symlen) {
	 // 锟斤拷
    char txtHead[3] = {(char)239,(char)187,(char)191};

    // Check if the token is the UTF8's magic word
    if(len == 3 && memcmp(token,txtHead,sizeof(char)*3) == 0){
        //check is 0xFEFF
		return true;
	}

	char result[256];
	sprintf(result, "%*.*s", symlen, symlen, token);
	if(str_eql(result, "") || str_eql(result, " ") || str_eql(result, "\t")) {
		return true;
	}

	if(isStopWord(result)) {
		return true;
	}

	return false;
}

bool Parser::peek(token_peek &peek) {
	while(true) {
		u2 len = 0, symlen = 0;
		char* tok = (char*)seg->peekToken(len, symlen);
		seg->popToken(len);

		if(!tok || !*tok || !len) // We have reach the end.
			return false;

		// If we can still get the token
		if(!filterToken(tok, len, symlen)) {
			if(*tok == '\r' || *tok == '\n') { // The line break should be ignored
				peek.type = TOKEN_TYPE_LINE_BREAK;
			}
			else
				peek.type = TOKEN_TYPE_TOKEN;
			// We have found the token, let's stop the peek
			peek.tok = tok;	
			peek.len = len;
			peek.symlen = symlen;
			break;
		}
		else {
			// Then, peek again
			continue;
		}
	}
	return true;
}

TOKEN_TYPE Parser::next(char* result) {
	token_peek t = {
		NULL, 0, 0, NULL
	};
	if(peek(t)) {
		sprintf(result, "%*.*s", t.symlen, t.symlen, t.tok);
		return t.type;
	}
	return t.type;
}
