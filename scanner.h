#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

vector<string> tokens; //saved tokens to parser
vector<string> errors; //saved errors 

const vector<pair<string, string>> operators{
    pair<string,string>("OP_SUM","+"), pair<string,string>("OP_RES","-"), pair<string,string>("OP_MULT","*"),
    pair<string,string>("OP_DIV","/"), pair<string,string>("OP_ASIG",":="), pair<string,string>("OP_EQU","="),
    pair<string,string>("OP_NOT_EQU","<>"), pair<string,string>("OP_LT","<"), pair<string,string>("OP_LE","<="),
    pair<string,string>("OP_GE",">="), pair<string,string>("OP_GT",">"), pair<string,string>("OP_HAT","^"),
    pair<string,string>("OP_AND","and"), pair<string,string>("OP_OR","or"), pair<string,string>("OP_NOT","not"),
    pair<string,string>("OP_DIV_ENT","div"), pair<string,string>("OP_MOD","mod"), pair<string,string>("OP_IN","in"),
    pair<string,string>("OP_DOT",".")
};
const vector<pair<string, string>> delimiters{
    pair<string,string>("DEL_COM",","), pair<string,string>("DEL_SEMC",";"), pair<string,string>("DEL_COL",":"),
    pair<string,string>("DEL_OP","("), pair<string,string>("DEL_CP",")"), pair<string,string>("DEL_OB","["),
    pair<string,string>("DEL_CBT","]"), pair<string,string>("DEL_DD","..")
};
const vector<pair<string, string>> reserved_words{
    pair<string,string>("ARRAY","array"), pair<string,string>("DOWNTO","downto"), pair<string,string>("FUNCTION","function"),
    pair<string,string>("OF","of"), pair<string,string>("REPEAT","repeat"), pair<string,string>("UNTIL","until"),
    pair<string,string>("BEGIN","begin"), pair<string,string>("ELSE","else"), pair<string,string>("GOTO","goto"),
    pair<string,string>("PACKED","packed"), pair<string,string>("SET","set"), pair<string,string>("VAR","var"),
    pair<string,string>("CASE","case"), pair<string,string>("END","end"), pair<string,string>("IF","if"),
    pair<string,string>("PROCEDURE","procedure"), pair<string,string>("THEN","then"), pair<string,string>("WHILE","while"),
    pair<string,string>("CONST","const"), pair<string,string>("FILE","file"), pair<string,string>("LABEL","label"),
    pair<string,string>("PROGRAM","program"), pair<string,string>("TO","to"), pair<string,string>("WITH","with"),
    pair<string,string>("DO","do"), pair<string,string>("FOR","for"), pair<string,string>("NIL","nil"),
    pair<string,string>("RECORD","record"), pair<string,string>("TYPE","type")
};

string file, context = "";

char get_char() { //consume the first char and move to the second
    context += file[0];
    file.erase(0, 1); //consume first || erase first
    return file[0];
}
bool peek_char(char next) { //see the next char but not consume it
    return (next == file[1]);
}
char peek_char() {
    return file[1];
}
bool peek2_char(char next) { //see the next next char but not consume it
    return (next == file[2]);
}
char peek2_char() {
    return file[2];
}
void skip_comment() {//not consume
    //skip until *) pr }
    if (file[0] == '(' && peek_char('*')) get_char(); //(
    char ch = get_char(); //{ || *
    while (ch) { //# 
        if (ch == '*' && peek_char(')')) {
            get_char(); get_char();
            context = "";  return; //not consume, not save
        }
        if (ch == '}') {
            get_char(); context = "";  return; //not consume, not save
        }
        ch = get_char();
    }
}
bool skip_space() { //space, tab or endLine
    char ch = file[0]; bool yes = 0;
    while (ch == ' ' || ch == '    ' || ch == '\t' || ch == '\v' || ch == '\f' || ch == '\n' || ch == 13)
        ch = get_char(), yes = 1; //not consume 

    if (ch == '\0') file = "", yes = 1;
    if (yes) context = "";
    return yes;
}
string convertNumExpDouble(double num) { //num exp [signo]* to double
    ostringstream ss;
    ss.precision(1);
    ss << std::fixed << num;
    return to_string(stod(ss.str()));
}
string convertNumDouble(string num) { //double to string
    return to_string(stod(num));
}
pair<int, string> isNumber() {//convert to number
    char ch = get_char(); //context fills
    while (true) {
        if ((ch == 'e' && (peek_char('+') || peek_char('-'))) || (ch == 'e' && isdigit(peek_char()))) { //exponente decimal con signo/sin signo
            ch = get_char(); //for e
            if (ch == '+' || ch == '-') ch = get_char(); //for + || -
            while (isdigit(ch)) ch = get_char(); //context filled

            if (isalpha(ch)) { //continue no matter a letter
                while (isalpha(peek_char()) || isdigit(peek_char())) //fill context to error
                    get_char();
                get_char();
                return pair<int, string>(7, "Unexpected letter in number " + context);
            }
            if (isdigit(context[context.size() - 1])) //expo signo
                return pair<int, string>(8, convertNumExpDouble(stod(context))); //to double
            else return pair<int, string>(7, "Unexpected number " + context);
        }
        if (isalpha(ch)) { //continue no matter a letter
            while (isalpha(peek_char()) || isdigit(peek_char())) //fill context to error
                ch = get_char();
            ch = get_char();
            return pair<int, string>(7, "Unexpected letter in number " + context);
        }
        if (!isdigit(ch) && !isalpha(ch)) break;
        ch = get_char(); //context fills
    }
    if (ch != '.' || (ch == '.' && peek_char('.')))
        return pair<int, string>(3, convertNumDouble(context)); //int // if continue ..
    ch = get_char(); //for .
    //For float (with .)
    while (true) {
        if ((ch == 'e' && (peek_char('+') || peek_char('-'))) || (ch == 'e' && isdigit(peek_char()))) { //exponente decimal con signo/sin signo
            ch = get_char(); //for e
            if (ch == '+' || ch == '-') ch = get_char(); //for + || -
            while (isdigit(ch)) ch = get_char(); //context filled

            if (isalpha(ch)) { //continue no matter a letter
                while (isalpha(peek_char()) || isdigit(peek_char())) //fill context to error
                    get_char();
                get_char();
                return pair<int, string>(7, "Unexpected letter in number " + context);
            }
            if (isdigit(context[context.size() - 1])) //expo signo
                return pair<int, string>(8, convertNumExpDouble(stod(context))); //to double
            else return pair<int, string>(7, "Unexpected number " + context);
        }
        if (isalpha(ch)) { //continue no matter a letter
            while (isalpha(peek_char()) || isdigit(peek_char())) //fill context to error
                get_char();
            get_char();
            return pair<int, string>(7, "Unexpected letter in double " + context);
        }
        if (!isdigit(ch) && !isalpha(ch)) break;
        ch = get_char();
    }
    return pair<int, string>(8, convertNumDouble(context)); //to double
}
pair<int, string> isWord() { //To know if is {Reserved_word || ID || operator} (maybe)
    char ch = file[0];
    if ((!peek_char('\0') && !isalpha(peek_char()) && !isdigit(peek_char())) || peek_char('\0')) {
        get_char(); context = "";
        string temp = ""; temp += ch;
        return pair<int, string>(4, temp); //only 1 letter is ID
    }

    if (isdigit(peek_char())) { //ID 
        ch = get_char();
        while (isdigit(ch) || isalpha(ch)) ch = get_char();
        return pair<int, string>(4, context); //ID
    }
    //{Reserved_word || ID || operator} (maybe)
    while (isalpha(peek_char())) //take letters until other symbol
        get_char();
    if (isdigit(peek_char())) { //ID
        ch = get_char();
        while (isdigit(ch) || isalpha(ch)) ch = get_char();
        return pair<int, string>(4, context); //ID
    }
    if (!isalpha(peek_char()) && !isdigit(peek_char())) { //find word
        int opt = -1;
        get_char(); //take last letter
        for (const auto& vect : operators)
            if (vect.second == context) {
                opt = 0; break; //OPERATOR (and or div ...)
            }
        if (opt == -1) { //continue looking for
            for (const auto& vect : reserved_words)
                if (vect.second == context) {
                    opt = 2; break; //RESERVED_WORD
                }
        }
        if (!opt || opt == 2)
            return pair<int, string>(opt, context); //from vectors

        return pair<int, string>(4, context); //ID, if not found in vectors
    }
}
pair<int, string> isString() {
    char ch = get_char(); context = ""; //do not include '
    if (ch == '\0' || ch == '\n') return pair<int, string>(7, "Invalid ' in unexpected end position");
    while (ch != '\0' && ch != '\'') //when is not '
        ch = get_char();
    if (!peek_char('\0') && peek_char('\'')) {//when is '', take one
        get_char();
        string contx = context;
        ch = get_char(); context = "";
        while (ch != '\0' && ch != '\'') //when is not ''
            ch = get_char();
        context = contx + context;
        if (ch == '\0') return pair<int, string>(7, "Missing end ' in " + context);
    }
    get_char(); context = context.erase(context.length() - 1); //final ' not
    return pair<int, string>(5, context);
}
pair<int, string> scanner() {
    context = "";
    //skip space/tab/endline
    if (skip_space()) return pair<int, string>(-1, "");
    //skip comment
    char ch = file[0];
    if (ch == '{' || (ch == '(' && peek_char('*'))) { //comment starts with { or (*
        skip_comment(); return pair<int, string>(-1, "");
    }

    int opt = -1; context = "";
    ch = file[0];
    switch (ch) {
        //Look for operators opt = 0
    case '+': opt = 0; get_char(); break;
    case '-': opt = 0; get_char(); break;
    case '*': opt = 0; get_char(); break;
    case '/': opt = 0; get_char(); break;
    case ':':
        if (peek_char('=')) get_char(), opt = 0; //operator opt = 0
        else opt = 1; //delimiter opt = 1
        get_char();  break;
    case '=': opt = 0; get_char(); break;
    case '<':
        if (peek_char('>')) get_char();
        else if (peek_char('=')) get_char();
        get_char(); opt = 0; break;
    case '>':
        if (peek_char('=')) get_char();
        opt = 0; get_char(); break;
    case '^': opt = 0; get_char(); break;
        //Look for delimiters opt = 1
    case ',': opt = 1; get_char(); break;
    case ';': opt = 1; get_char(); break;
    case '(': opt = 1; get_char(); break;
    case ')': opt = 1; get_char(); break;
    case '[': opt = 1; get_char(); break;
    case ']': opt = 1; get_char(); break;
    case '.':
        if (peek_char('.')) get_char(), opt = 1; //delimiter
        else opt = 0;
        get_char(); break;
    default:
        break;
    };
    if (opt == 1 || !opt) return pair<int, string>(opt, context);
    context = ""; ch = file[0];
    //Look for number
    if (isdigit(ch))
        return isNumber();

    if (isalpha(ch)) //Look for reserved_word or ID or operator
        return isWord();

    if (ch == '\'') //Look for string
        return isString();

    get_char();
    return pair<int, string>(7, "Unknown symbol:" + context);
}
string gettoken() { //put token in tokens_vector or error in errors_vector
    pair<int, string> data = scanner(); //call scan
    if (data.first == -1) return "";
    string tag, st;
    if (!data.first) {
        for (const auto& vect : operators)
            if (vect.second == data.second) tag = vect.first;
    }
    else if (data.first == 1) {
        for (const auto& vect : delimiters)
            if (vect.second == data.second) tag = vect.first;
    }
    else if (data.first == 2) {
        for (const auto& vect : reserved_words)
            if (vect.second == data.second) tag = vect.first;
    }
    else if (data.first == 3)  tag = "NUM_INT";
    else if (data.first == 4) tag = "ID";
    else if (data.first == 5) tag = "STRING";
    else if (data.first == 6) tag = "EOF";
    else if (data.first == 7) {
        st = "<error," + data.second + ">";
        errors.push_back(st); return st;
    }
    else if (data.first == 8) tag = "NUM_REAL";
    else { cout << data.first << endl; return to_string(data.first); }
    st = "<" + tag + "," + data.second + ">";
    tokens.push_back(st); return st;
}
void get(const string& nameFile) { //get all tokesn and errors from the file
    fstream ff;
    ff.open(nameFile, ios::in);
    if (!ff) { cout << "Error open file" << endl; return; }

    string temp; file = "";
    char byte = 0;
    while (std::getline(ff, temp))
        file += temp + '\n';
    ff.close();

    while (!file.empty()) {
        temp = gettoken();
        if (!temp.empty()) cout << temp << " "; //call gettoken to have tokens and errors
    }
    string finalTok = "<EOF,$>";
    tokens.push_back(finalTok); cout << finalTok << endl;
}