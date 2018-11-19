#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;

int main () {
	string text;
	getline (cin, text);
        while (text[0] == ' ' || text[0] == '\t') {
                text.erase(0,1);
	}
        int i = min(text.find(' '),text.find('\t'));
        while (i != -1) {
                while (text[i+1] == ' ' || text[i+1] == '\t') {
			text.erase(i+1,1);
		}
                i = min(text.find(' ', i+1),text.find('\t', i+1));
	}
        if (text[text.length() - 1] == ' ' || text[text.length() - 1] == '\t') {
		text.erase(text.length() - 1,1);
	}
        i  = min(text.find(' '),text.find('\t'));
	string word;
	vector <string> myvector;
	while(i != -1) {
		word = text.substr(0,i);
		myvector.push_back(word);
		text.erase(0,i+1);
                i  = min(text.find(' '),text.find('\t'));
	}
	i = text.length();
	if (i > 0) {
		word = text.substr(0,i);
		myvector.push_back(word);
		text.erase(0,i);
	}
	vector <char *> argv;
	for (i = 0; i < myvector.size(); i++) {
		argv.push_back((char *)myvector[i].c_str());
	}
	argv.push_back(NULL);



	execvp(argv[0], &argv[0]);
	return 0;
}
