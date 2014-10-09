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
	param->flags = MYSQL_FTFLAGS_NEED_COPY; // Since we just using 1 buffer to add indexes, so mysql need to copy the text.
    set_status("Tokenizing the string");
	parser->logger->log("Tokenizing the string");

	TOKEN_TYPE t;
	char tok[1024];
	char* operator_type = NULL;

	parser->set(param->doc, param->length);
	while((t = parser->next(tok))) {
		MYSQL_FTPARSER_BOOLEAN_INFO *info = NULL;
		if(param->mode != MYSQL_FTPARSER_SIMPLE_MODE || t == TOKEN_TYPE_TOKEN) {
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
			if(param->mode == MYSQL_FTPARSER_FULL_BOOLEAN_INFO) {
				// Only in full boolean mode, will test if the token is the operator

				// Testing the token if the token is the operator
				if(str_eql(tok, OPERATOR_TYPE_PLUS)) {
					operator_type = OPERATOR_TYPE_PLUS;
					continue;
				}

				if(str_eql(tok, OPERATOR_TYPE_MINUS)) {
					operator_type = OPERATOR_TYPE_MINUS;
					continue;
				}

				if(str_eql(tok, OPERATOR_TYPE_GT)) {
					operator_type = OPERATOR_TYPE_GT;
					continue;
				}

				if(str_eql(tok, OPERATOR_TYPE_LT)) {
					operator_type = OPERATOR_TYPE_LT;
					continue;
				}

				if(str_eql(tok, OPERATOR_TYPE_TILDE)) {
					operator_type = OPERATOR_TYPE_TILDE;
					continue;
				}

				if(str_eql(tok, OPERATOR_TYPE_ASTERISK)) {
					operator_type = OPERATOR_TYPE_ASTERISK;
					continue;
				}

				// Find the word's type first
				if(str_eql(tok, OPERATOR_TYPE_LP)) {
					bool_info.type = FT_TOKEN_LEFT_PAREN;
				} else if(str_eql(tok, OPERATOR_TYPE_RP)) {
					bool_info.type = FT_TOKEN_RIGHT_PAREN;
				} else if(t == TOKEN_TYPE_STOP_WORD) {
					bool_info.type = FT_TOKEN_STOPWORD;
				}

				// Then adjusting the bool info now
				if(operator_type) {
					if(str_eql(operator_type, OPERATOR_TYPE_PLUS)) {
						bool_info.yesno = 1; // This word is must have
					}

					if(str_eql(operator_type, OPERATOR_TYPE_MINUS)) {
						bool_info.yesno = -1; // This word is must not have
					}

					if(str_eql(operator_type, OPERATOR_TYPE_GT)) {
						bool_info.weight_adjust = 1; // This word will be increase the relavance of the search
					}

					if(str_eql(operator_type, OPERATOR_TYPE_LT)) {
						bool_info.weight_adjust = -1; // This word will be decrease the relavance of the search
					}

					if(str_eql(operator_type, OPERATOR_TYPE_TILDE)) {
						bool_info.wasign = -1;
					}

					if(str_eql(operator_type, OPERATOR_TYPE_ASTERISK)) {
						bool_info.trunc = 1;
					}
				}

				info = &bool_info;
			}
			param->mysql_add_word(param, tok, strlen(tok), info);
			operator_type = NULL; // Reset the operator type after the operation
		}
	}

    return 0;
}
int main(int argc, char *argv[]) {
	std::string data = "test/data.txt";
	if(argc >= 2) {
		data = argv[1];
	}
	std::cout << "File name is " << data << std::endl;
	std::auto_ptr<INIReader> reader_ptr(read_config());
	reader = reader_ptr.get();

	if(!reader)
		return -1;

	Logger logger;

	logger.log("Reading the testing file.");
    std::istream *is = new std::ifstream(data.c_str(), std::ifstream::in | std::ifstream::binary);
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
		if(t == TOKEN_TYPE_STOP_WORD) {
			std::cout << result << "/s ";
			continue;
		}
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
	u2 len = 0, symlen = 0;
	
	// Getting the token
	char* tok = (char*)seg->peekToken(len, symlen);
	seg->popToken(len);

	if(!tok || !*tok || !len) // We have reach the end.
		return false;

	// If we can still get the token
	peek.tok = tok;	
	peek.len = len;
	peek.symlen = symlen;
	if(*tok == '\r' || *tok == '\n') { // The line break should be ignored
		peek.type = TOKEN_TYPE_LINE_BREAK;
	}
	else {
		if(!filterToken(tok, len, symlen)) {
			peek.type = TOKEN_TYPE_TOKEN;
		}
		else {
			peek.type = TOKEN_TYPE_STOP_WORD;
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
