#include "helper.h"

// source: https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
vector<string> helper::splitToVector(string str, char del)
{
	vector<string> splitResult;

	// declaring temp string to store the curr "word" upto del
	string temp = "";

	for (int i = 0; i < (int)str.size(); i++) {
		// If cur char is not del, then append it to the cur "word", otherwise
		  // you have completed the word, print it, and start a new word.
		if (str[i] != del) {
			temp += str[i];
		}
		else {
			splitResult.push_back(temp);
			temp = "";
		}
	}

	splitResult.push_back(temp);

	return splitResult;
}