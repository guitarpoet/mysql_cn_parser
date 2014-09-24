#include <iostream>
#include <fstream>
#include <vector>


std::vector<std::string>* stopwords = NULL;

void readStopWords(std::string stopWordsFile) {
	stopwords = new std::vector<std::string>();
	std::ifstream _file(stopWordsFile);
	std::copy(std::istream_iterator<std::string>(_file),
	std::istream_iterator<std::string>(),
	std::back_inserter(*stopwords));
}

bool isStopWord(std::string item) {
	return std::find(stopwords->begin(), stopwords->end(), item) != stopwords->end();
}



int main() {
	readStopWords("stop_words.txt");
	std::cout << stopwords->size() << std::endl;
	std::cout << isStopWord("'") << std::endl;
	std::cout << isStopWord("<") << std::endl;
	delete stopwords;
	char result[16];
	sprintf(result, "%*.*s", 1, 1, " asdf");
	std::cout << "--" << result << "--" << std::endl;
	if(strncmp(result, " ", sizeof(" ") - 1) == 0)
		std::cout << "Haha" << std::endl;
}
