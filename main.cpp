#include "scanner.h"
#include "parser.h"
#include <iostream>

using namespace std;

int main() {
	cout << "------------------------------SCANNER------------------------------" << endl;
	get("../programa.txt");
	get_vector(tokens);
	cout << "------------------------------PARSER------------------------------" << endl;
	parser();	
	return 0;
}