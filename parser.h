#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "run.h";
using namespace std;

//PARSE TREE
struct PNode {
    PNode(string _v, int _space) {
        value = _v; space = _space; //number od spaces to print
    }
    ~PNode() {
        nodes.clear();
    }
    int cont_spaces() { //cont spaces to print me
        int i = 0, s = nodes.size(), cont = 0;
        for (; i < s; i++)
            cont += nodes[i]->value.size() + 1; //strings + spaces of separation
        return space + cont; //num_spaces of dad and mine
    }
    string value; int space; vector<PNode*> nodes;
};
class ParseTree {
public:
    PNode* root;
    ParseTree() { root = new PNode("-START-", 0); }
    ~ParseTree() {
        delTree(root);
    }
    void delTree(PNode*& r) {
        if (!r) return;
        for (int temp = 0; temp < r->nodes.size(); temp++)
            delTree(r->nodes[temp]);
        delete r; r = NULL;
        return;
    }
    void insert(PNode* dad, string child) {
        dad->nodes.push_back(new PNode(child, dad->cont_spaces()));
    }
};
struct Generator {
    string code = "";
    void emit(string code_) {
        code += code_;
    }
    void run_code() {
        cout << code << endl;
        ofstream out_file;
        out_file.open("run.h");
        string code_ = code.substr(0, code.find("main") - 1);
        out_file << code_ <<" runCode ";
        code_ = code.substr(code.find("("));
        out_file << code_;
        out_file.close();
        cout << "------------------------------RUN_CODE------------------------------" << endl;
        //Run code twice to see the real print, not the last
        runCode();
        cout << endl;
    }
};
struct Data_var {
    string name = "", type = "", value = "";
    bool var; //1:var , 0:const
    //Data_var() {}
    Data_var(string name_, string type_, string value_, bool var_) {
        name = name_; type = type_; value = value_;
        var = var_;
    }
};
Generator generator;
ParseTree tree;
int num_token = 0;
string token_tag[2]; //token to see
vector<string> tags;
vector<string> errors_parser;

void get_vector(vector<string> vec) { //copy tokens
    for (int i = 0; i < vec.size(); i++)
        tags.push_back(vec[i]); //token
}
void next_token() { //next token to see the production of grammar
    token_tag[0] = tags[num_token].substr(1, tags[num_token].find(',') - 1); //tag
    token_tag[1] = tags[num_token].substr(tags[num_token].find(',') + 1); //value
    token_tag[1] = token_tag[1].substr(0, token_tag[1].size() - 1);
    cout << token_tag[0] << "    " << token_tag[1] << endl; num_token++; //to know in which part is and look if is an error
}
void add_error(string error, string* follows, int size_flls) {
    errors_parser.push_back(error);
    cout << error << endl;
    while (token_tag[0] != "EOF") {
        for (int i = 0; i < size_flls; i++) {
            if (token_tag[0] == *(follows + i) || token_tag[1] == *(follows + i)) return;
        } next_token();
    } return;
}
void add_node(PNode* dad, string child) { //create token_node in the tree
    tree.insert(dad, child);
}
void print_parse_tree(PNode* node) { //Print recursively parse tree 
    if (node->nodes.empty()) return; //is terminal --> return, else continue
    //cout << node->value << "    ->  ";
    int s = node->nodes.size(), temp = node->space; //print spaces of dad and mine
    while (temp > 0) cout << ' ', temp--;
    cout << "|"; //to know which nonterminal it corresponds to 
    for (temp = 0; temp < s; temp++) //print children
        cout << node->nodes[temp]->value << ' ';
    cout << endl;
    for (temp = 0; temp < s; temp++) //function to children
        print_parse_tree(node->nodes[temp]);
}
//Struct parser
struct parser_class {
    vector<Data_var> data_vector;
    bool found(string n, string& return_type) {
        for (int i = 0; i < data_vector.size(); i++)
            if (data_vector[i].name == n) { return_type = data_vector[i].type; return 1; }
        return 0;
    }
    //functions for each no-terminal
    void Program() { //Program → PROGRAM id ; ConstBlock VarBlock MainCode
        //Follow is CONST,VAR,BEGIN,EOF
        string follows[] = { "CONST","VAR","BEGIN" };
        generator.emit("#include <iostream> \nusing namespace std; \nvoid main () { \n"); //C++ translation
        if (token_tag[0] != "PROGRAM") add_error("ErrorProgram: Find '" + token_tag[1] + "' but it must be 'program'.", follows, 3);
        else {
            add_node(tree.root, "program"); next_token(); 
            if (token_tag[0] != "ID") add_error("ErrorProgram: Find '" + token_tag[1] + "' but it must be 'id'.", follows, 3);
            else { add_node(tree.root, "id"); next_token(); }
            if (token_tag[0] != "DEL_SEMC") add_error("ErrorProgram: Find '" + token_tag[1] + "' but it must be ';'.", follows, 3);
            else { add_node(tree.root, ";"); next_token(); } 
        }
        ConstBlock(tree.root); VarBlock(tree.root); 
        MainCode(tree.root); //Obligatorio
        generator.emit("\n}\n"); //C++ translation
    }
    void ConstBlock(PNode* dad) { //ConstBlock → CONST ConstList | Ɛ
        //Follow is VAR,BEGIN
        if (token_tag[0] != "CONST") return; //Ɛ
        add_node(dad, "const"); next_token();
        return ConstList(dad);
    }
    void ConstList(PNode* dad) { //ConstList → id = Value; ConstList | id = Value;
        add_node(dad, "ConstList"); //Follow is VAR,BEGIN
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "VAR","BEGIN" };
        if (token_tag[0] != "ID") return add_error("ErrorConstList: Find '" + token_tag[1] + "' but it must be 'id'.", follows, 2);
        add_node(new_Dad, "id"); string nombre = token_tag[1]; next_token();
        if (token_tag[0] != "OP_EQU") return add_error("ErrorConstList: Find '" + token_tag[1] + "' but it must be '='.", follows, 2);
        add_node(new_Dad, "="); next_token();
        pair<string, string> val;
        Value(new_Dad, val); //already next_token
        data_vector.push_back(Data_var(nombre, val.first, val.second, 0)); //Save name, type, value, const of Value()
        generator.emit("const ");
        if (val.first == "NUM_INT") generator.emit("int ");
        else if (val.first == "NUM_REAL") generator.emit("double ");
        else if (val.first == "STRING") generator.emit("string ");
        generator.emit(nombre + " = " + val.second + "; \n");
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorConstList: Find '" + token_tag[1] + "' but it must be ';'.", follows, 2);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return ConstList(new_Dad); //Recursion
    }
    void Value(PNode* dad, pair<string,string>& val) { //Value → INTEGER | REAL | STRING 
        add_node(dad, "Value"); //Follow is ';',TO,DOWNTO,MUL,DIV,MOD,AND,+,-,OR,=,<>,<,<=,>=,>,),DO,','
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "TO","DOWNTO","OP_MULT","OP_DIV","OP_DIV_ENT","OP_MOD","OP_AND","OP_SUM","OP_RES","OP_OR","OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DO","DEL_SEMC","DEL_COM","DEL_CP" };
        if (token_tag[0] != "NUM_INT" && token_tag[0] != "NUM_REAL" && token_tag[0] != "STRING" && token_tag[1] != "integer" && token_tag[1] != "real" && token_tag[1] != "string")
            return add_error("ErrorValue: Find '" + token_tag[1] + "' but it must be 'v_int' or 'v_real' or 'v_string' or 'integer' or 'real' or 'string'.", follows, follows->size());
        add_node(new_Dad, "v_" + token_tag[0]); 
        val.first = token_tag[0]; val.second = token_tag[1]; //type and value
        next_token();
    }
    void NameList(PNode* dad, vector<string>& names) { //NameList → id | id, NameList
        add_node(dad, "NameList"); //Follow is :
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_COL" };
        if (token_tag[0] != "ID") return add_error("ErrorNameList: Find '" + token_tag[1] + "' but it must be 'id'.", follows, 1);
        add_node(new_Dad, "id"); names.push_back(token_tag[1]); next_token();
        if (token_tag[0] != "DEL_COM") return;
        add_node(new_Dad, ","); next_token();
        return NameList(new_Dad, names); //Recursion
    }
    void VarBlock(PNode* dad) { //VarBlock → VAR VarList | Ɛ
        //Follow is BEGIN
        if (token_tag[0] != "VAR") return; //Ɛ
        add_node(dad, "VarBlock");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad, "var"); next_token();
        return VarList(new_Dad);
    }
    void VarList(PNode* dad) { //VarList → NameList : TypeDecl; VarList | Ɛ
        add_node(dad, "VarList"); //Follow is BEGIN
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "BEGIN" };
        vector<string> names;
        NameList(new_Dad, names);
        if (token_tag[0] != "DEL_COL") return add_error("ErrorVarList: Find '" + token_tag[1] + "' but it must be ':'.", follows, 1);
        add_node(new_Dad, ":"); next_token();
        pair<string, string> val;
        Value(new_Dad, val); //already next_token
        if (val.first == "NUM_INT" || val.second == "integer") generator.emit("int "), val.first = "NUM_INT";
        else if (val.first == "NUM_REAL" || val.second == "real") generator.emit("double "), val.first = "NUM_REAL";
        else if (val.first == "STRING" || val.second == "string") generator.emit("string "), val.first = "STRING";
        generator.emit(names[0]);
        for (int i = 0; i < names.size(); i++) {
            data_vector.push_back(Data_var(names[i], val.first, "", 1)); //Save name, type, value, var of TypeDecl() //value is empty(to be modified later in other functions like assign)
            if (i != 0) generator.emit(", " + names[i]);
        } generator.emit("; \n");
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorVarList: Find '" + token_tag[1] + "' but it must be ';'.", follows, 1);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return VarList(new_Dad); //Recursion
    }
    void StmtList(PNode* dad) { //StmtList → Stmt StmtList | e
        add_node(dad, "StmtList"); //Follow is END
        PNode* new_Dad = dad->nodes.back();
        Stmt(new_Dad);
        if (token_tag[0] == "ID" || token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln" ||
            token_tag[0] == "IF" || token_tag[0] == "FOR" ||token_tag[1] == "Write" || token_tag[1] == "Writeln") return StmtList(new_Dad); //Recursion
    }
    void Stmt(PNode* dad) { //Stmt → break; | continue; | Assign | Proc | IfBlock ElseBlock| ForBlock 
        add_node(dad, "Stmt"); //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "END","ID","IF","FOR","break","continue","read","readln","write","Write","writeln","Writeln" };
        if (token_tag[1] == "break" || token_tag[1] == "continue") {
            add_node(new_Dad, token_tag[1]);  generator.emit(token_tag[1] + ";\n"); next_token();
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        if (token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln" || token_tag[1] == "Write" || token_tag[1] == "Writeln") 
            return Proc(new_Dad); //already next token
        if (token_tag[0] == "ID") return Assign(new_Dad); //already next token
        if (token_tag[0] == "IF") {
            IfBlock(new_Dad); //already next token
            if (token_tag[0] == "ELSE") return ElseBlock(new_Dad);
            else {
                if (token_tag[0] != "DEL_SEMC") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
                add_node(new_Dad, ";"); next_token();
            } return;
        }
        if (token_tag[0] == "FOR") return ForBlock(new_Dad);
        return add_error("ErrorStmt: Find '" + token_tag[1] + "' but it must be 'break' or 'continue' or 'id' or 'read' or 'readln' or 'write' or 'Write' or 'writeln' or 'Writeln' or 'if' or 'for'.", follows, follows->size());
    }
    string type_assign = ""; bool equal_assign = 0;
    void Assign(PNode* dad) { //Assign → ID := Expr;
        add_node(dad, "Assign"); //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "IF","FOR","break","continue","read","readln","write","Write","writeln","Writeln","ID","END" };
        if (token_tag[0] != "ID") return add_error("ErrorAssign: Find '" + token_tag[1] + "' but it must be 'id'.", follows, follows->size());
        string type; add_node(new_Dad, "id");
        if (!found(token_tag[1], type)) //Error declaration
            return add_error("ErrorAssign: Find id:'" + token_tag[1] + "' , but it must be declared first.", follows, follows->size());
        generator.emit(token_tag[1]); next_token();
        if (token_tag[0] != "OP_ASIG") return add_error("ErrorAssign: Find '" + token_tag[1] + "' but it must be ':='.", follows, follows->size());
        add_node(new_Dad, ":="); next_token(); generator.emit(" = "); 
        Expr(new_Dad, type_assign); //Emit in the function 
        if (type != type_assign || !equal_assign) return add_error("ErrorAssign: Find different types of id and Expr.", follows, follows->size());
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorAssign: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
        add_node(new_Dad, ";"); next_token(); generator.emit(" ; \n");
    }
    void Proc(PNode* dad) { //Proc → READ (NameList); | READLN (NameList); | WRITE args; | WRITELN args;
        add_node(dad, "Proc"); //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "IF","ID","FOR","break","continue","read","readln","write","Write","writeln","Writeln","END" };
        if (token_tag[1] != "read" && token_tag[1] != "readln" && token_tag[1] != "write" && token_tag[1] != "Write" && token_tag[1] != "writeln" && token_tag[1] != "Writeln")
            return add_error("ErrorProc: Find '" + token_tag[1] + "' but it must be 'read' or 'readln' or 'write' or 'Write' or 'writeln' or 'Writeln'.", follows, 1);
        add_node(new_Dad, token_tag[1]); string content_token = token_tag[1]; next_token();
        if (content_token == "write" || content_token == "Write" || content_token == "writeln" || content_token == "Writeln") {  //C++ translation
            generator.emit("cout << ");
            args(new_Dad);
            if (content_token == "writeln" || content_token == "Writeln") generator.emit("<< endl;\n"); //C++ translation
            else generator.emit(";\n");
        }
        else { // args → (NameList) //READ,READLN
            if (token_tag[0] != "DEL_OP") return add_error("ErrorArgs: Find '" + token_tag[1] + "' but it must be '('.", follows, 1);
            add_node(new_Dad, "("); next_token();
            generator.emit("cin << (");
            vector<string> names;
            NameList(new_Dad, names);
            generator.emit(names[0]);
            for (int i = 1; i < names.size(); i++)
                generator.emit(", " + names[i]);
            generator.emit(");\n");
            if (token_tag[0] != "DEL_CP") return add_error("ErrorArgs: Find '" + token_tag[1] + "' but it must be ')'.", follows, 1);
            add_node(new_Dad, ")"); next_token();
        }
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorProc: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
        add_node(new_Dad, ";"); next_token();
    }
    void args(PNode* dad) { //args → (ExprList) //Simplified to: args->(string|num_int|num_real|id)
        add_node(dad, "Args"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC" };
        if (token_tag[0] != "DEL_OP") return add_error("ErrorArgs: Find '" + token_tag[1] + "' but it must be '('.", follows, 1);
        add_node(new_Dad, "("); next_token(); generator.emit("( ");
        ExprList(new_Dad);
        generator.emit(") ");
        if (token_tag[0] != "DEL_CP") return add_error("ErrorArgs: Find '" + token_tag[1] + "' but it must be ')'.", follows, 1);
        add_node(new_Dad, ")"); next_token(); 
    }
    void IfBlock(PNode* dad) { //IfBlock → IF (Expr) THEN BEGIN StmtList END ElseBlock
        add_node(dad, "IfBlock"); //Follow is ELSE | END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR 
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "ELSE" }; //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        if (token_tag[0] != "IF") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be 'if'.", follows, follows->size());
        generator.emit("if (");
        add_node(new_Dad, "if"); next_token(); 
        if (token_tag[0] != "DEL_OP") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be '('.", follows, follows->size());
        add_node(new_Dad, "("); next_token();
        Expr(new_Dad, type_assign); //verify inside that function and emit
        if (token_tag[0] != "DEL_CP") return add_error("ErrorFactor: Find '" + token_tag[1] + "' but it must be ')'.", follows, follows->size());
        add_node(new_Dad, ")"); next_token();  generator.emit(") { \n");
        if (token_tag[0] != "THEN") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be 'then'.", follows, follows->size());
        add_node(new_Dad, "then"); next_token(); 
        if (token_tag[0] != "BEGIN") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be 'begin'.", follows, follows->size());
        add_node(new_Dad, "begin"); next_token(); 
        StmtList(new_Dad); //verify inside that function and emit
        if (token_tag[0] != "END") return add_error("ErrorIfBlock: Find '" + token_tag[1] + "' but it must be 'end'.", follows, follows->size());
        add_node(new_Dad, "end"), next_token(); generator.emit("} \n");
    }
    void ElseBlock(PNode* dad) { //ElseBlock → ELSE BEGIN StmtList END ;
        //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        add_node(dad, "ElseBlock");
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "IF","ID","FOR","break","continue","read","readln","write","Write","writeln","Writeln","END" };
        add_node(new_Dad, "else"); next_token();
        generator.emit("else { \n");
        if (token_tag[0] != "BEGIN") return add_error("ErrorElseBlock: Find '" + token_tag[1] + "' but it must be 'begin'.", follows, follows->size());
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad); //verify inside that function and emit
        if (token_tag[0] != "END") return add_error("ErrorElseBlock: Find '" + token_tag[1] + "' but it must be 'end'.", follows, follows->size());
        add_node(new_Dad, "end"), next_token();
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorElseBlock: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
        add_node(new_Dad, ";"); next_token(); generator.emit("} \n");
    }
    void ForBlock(PNode* dad) { //ForBlock → FOR id := Value Direction Expr DO BEGIN StmtList END ; ..... //Direction → TO | DOWNTO
        add_node(dad, "ForBlock"); //Follow is END,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,FOR
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "IF","FOR","break","continue","read","readln","write","Write","writeln","Writeln","ID","END" };
        if (token_tag[0] != "FOR") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'for'.", follows, follows->size());
        add_node(new_Dad, "for"); next_token();
        string type; generator.emit("for (");
        if (token_tag[0] != "ID") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'id'.", follows, follows->size());
        if (!found(token_tag[1], type)) //Error declaration
            return add_error("ErrorForBlock: Find id:'" + token_tag[1] + "' , but it must be declared first.", follows, follows->size()); 
        generator.emit(token_tag[1]); //Save name_id in the code to execute
        string id = token_tag[1];
        add_node(new_Dad, "id"); next_token();
        if (token_tag[0] != "OP_ASIG") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be ':='.", follows, follows->size());
        add_node(new_Dad, ":="); next_token(); generator.emit(" = "); //Save assign C++
        pair<string, string> val;
        Value(new_Dad, val);
        if (val.first != type) return add_error("ErrorForBlock: Find type:'" + val.first + "', but it must be the type " + type + '.', follows, follows->size()); //Error type
        generator.emit(val.second + "; ");
        if (token_tag[0] != "TO" && token_tag[0] != "DOWNTO") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'to' or 'downto'.", follows, follows->size());
        //to: <= , downto: >=
        bool to;
        if (token_tag[0] == "TO") generator.emit(id + " <= "), to = 1;
        else generator.emit(id + " >= "), to = 0;
        add_node(new_Dad, token_tag[0]); next_token();
        string type2;
        Expr(new_Dad, type2); //verify inside that function and emit
        if (type != type2) return add_error("ErrorForBlock: Find type:'" + type2 + "', but it must be the type " + type + '.', follows, follows->size()); //Error type
        if (to) generator.emit("; " + id + "++");
        else generator.emit("; " + id + "--");
        if (token_tag[0] != "DO") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'do'.", follows, follows->size());
        add_node(new_Dad, "do"); next_token(); generator.emit(") { \n");
        if (token_tag[0] != "BEGIN") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'begin'.", follows, follows->size());
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad); //verify inside that function and emit
        if (token_tag[0] != "END") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be 'end'.", follows, follows->size());
        add_node(new_Dad, "end"), next_token();
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorForBlock: Find '" + token_tag[1] + "' but it must be ';'.", follows, follows->size());
        add_node(new_Dad, ";"); next_token(); generator.emit("\n}\n");
    }
    void ExprList(PNode* dad) { //ExprList → Expr, ExprList | Expr
        add_node(dad, "ExprList"); //Follow is )
        PNode* new_Dad = dad->nodes.back();
        string type_1;
        Expr(new_Dad, type_1);
        if (token_tag[0] != "DEL_COM") return; //e
        add_node(new_Dad, ","); next_token(); generator.emit(", ");
        return ExprList(new_Dad); //Recursion
    }
    void Expr(PNode* dad, string& type_1) { //Expr → Expr_ RelOp Expr | Expr_
        add_node(dad, "Expr"); //Follow is ),;,DO,','
        PNode* new_Dad = dad->nodes.back();
        string follows[] = {"DO","DEL_SEMC","DEL_COM","DEL_CP"};
        Expr_(new_Dad, type_1);
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return;
        RelOp(new_Dad); 
        string type_2;
        Expr(new_Dad, type_2); //Recursion
        if (type_1 != type_2) { //Error type
            equal_assign = 0; return add_error("ErrorExpr: Find type:'" + type_2 + "', but it must be the type " + type_1 + '.', follows, follows->size());
        } equal_assign = 1;
    }
    void RelOp(PNode* dad) { //RelOp → = | <> | < | <= | >= | >
        add_node(dad, "RelOp"); //Follow is ID,NUM_INT,NUM_REAL,STRING,(,NOT,MINUS
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "ID","NUM_INT","NUM_REAL","STRING","DEL_OP","OP_NOT","OP_RES" };
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return add_error("ErrorRelOP: Find '" + token_tag[1] + "' but it must be '=' or '<>' or '<' or '<=' or '>=' or '>'.", follows, follows->size());
        add_node(new_Dad, token_tag[0]); generator.emit(token_tag[1] + ' '); next_token();
    }
    void Expr_(PNode* dad, string& type_1) { //Expr_ → Term + Expr_ | Term - Expr_ | Term OR Expr_ | Term
        add_node(dad, "Expr_"); //Follow is =,<>,<,<=,>=,>,),;,DO,','
        string follows[] = { "OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DO","DEL_SEMC","DEL_COM","DEL_CP" };
        PNode* new_Dad = dad->nodes.back();
        Term(new_Dad, type_1);
        if (token_tag[0] != "OP_SUM" && token_tag[0] != "OP_RES" && token_tag[0] != "OP_OR")
            return; //Term
        generator.emit(token_tag[1] + ' ');
        add_node(new_Dad, token_tag[0]); next_token();
        string type_2;
        Expr_(new_Dad, type_2); //Recursion
        if (type_1 != type_2) { //Error type
            equal_assign = 0; return add_error("ErrorExpr_: Find type:'" + type_2 + "', but it must be the type " + type_1 + '.', follows, follows->size());
        } equal_assign = 1;
    }
    void Term(PNode* dad, string& type_1) { //Term → Factor MUL|DIV|MOD|AND Term    |    Factor
        add_node(dad, "Term"); //Follow is +,-,OR,=,<>,<,<=,>=,>,),;,DO,','
        string follows[] = { "OP_SUM","OP_RES","OP_OR","OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DO","DEL_SEMC","DEL_COM","DEL_CP" };
        PNode* new_Dad = dad->nodes.back();
        Factor(new_Dad, type_1);
        if (token_tag[0] != "OP_MULT" && token_tag[0] != "OP_DIV" && token_tag[0] != "OP_DIV_ENT" && token_tag[0] != "OP_MOD" && token_tag[0] != "OP_AND")
            return; //Factor
        generator.emit(token_tag[1] + ' ');
        add_node(new_Dad, token_tag[0]); next_token();
        string type_2;
        Term(new_Dad, type_2); //Recursion
        if (type_1 != type_2) { //Error type
            equal_assign = 0; return add_error("ErrorTerm: Find type:'" + type_2 + "', but it must be the type " + type_1 + '.', follows, follows->size());
        } equal_assign = 1;
    }
    void Factor(PNode* dad, string& type_) { //Factor → ID | Value | (Expr) 
        add_node(dad, "Factor"); //Follow is MUL,DIV,MOD,AND,+,-,OR,=,<>,<,<=,>=,>,),;,DO,','
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "OP_MULT","OP_DIV","OP_DIV_ENT","OP_MOD","OP_AND","OP_SUM","OP_RES","OP_OR","OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DO","DEL_SEMC","DEL_COM","DEL_CP" };
        if (token_tag[0] == "ID" || token_tag[0] == "NUM_INT" || token_tag[0] == "NUM_REAL" || token_tag[0] == "STRING") { //ID | Value
            if (token_tag[0] == "ID" && !found(token_tag[1], type_)) return add_error("ErrorFactor: Find id:'" + token_tag[1] + "' , but it must be declared first.", follows, follows->size()); //Error declaration
            else if(token_tag[0] != "ID") type_ = token_tag[0];
            if (token_tag[0] == "STRING") generator.emit("\"" + token_tag[1] + "\" ");
            else generator.emit(token_tag[1] + ' ');
            add_node(new_Dad, "v_" + token_tag[0]); next_token(); equal_assign = 1;
            return;
        }
        if (token_tag[0] == "DEL_OP") { //(Expr)
            add_node(new_Dad, "("); next_token(); generator.emit("( ");
            Expr(new_Dad, type_);
            if (token_tag[0] != "DEL_CP") return add_error("ErrorFactor: Find '" + token_tag[1] + "' but it must be ')'.", follows, follows->size());
            add_node(new_Dad, ")"); next_token(); equal_assign = 1; generator.emit(" ) ");
            return;
        }
        return add_error("ErrorFactor: Find '" + token_tag[1] + "' but it must be 'id' or 'int' or 'real' or 'string' or 'function' or '(' or 'not' or '-'.", follows, follows->size());
    }
    void MainCode(PNode* dad) { //MainCode → BEGIN StmtList END.
        add_node(dad, "MainCode"); //Follow is EOF $
        PNode* new_Dad = dad->nodes.back(); string follows[] = { "EOF" };
        if (token_tag[0] != "BEGIN") return add_error("ErrorMainCode: Find '" + token_tag[1] + "' but it must be 'begin'.", follows, 1);
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad);
        if (token_tag[0] != "END") return add_error("ErrorMainCode: Find '" + token_tag[1] + "' but it must be 'end'.", follows, 1);
        add_node(new_Dad, "end"); next_token();
        if (token_tag[0] != "OP_DOT") return add_error("ErrorMainCode: Find '" + token_tag[1] + "' but it must be '.'.", follows, 1);
        add_node(new_Dad, "."); next_token();
    }
};
void parser() {
    next_token();
    parser_class parser;
    parser.Program();
    if (!errors_parser.empty()) { //Error
        cout << "------------------------------ERRORS------------------------------" << endl;
        int i = 0, s_e = errors_parser.size();
        for (; i < s_e; i++) cout << errors_parser[i] << endl;
        return;
    }
    //cout << "------------------------------PARSE_TREE------------------------------" << endl;
    //print_parse_tree(tree.root); //Print parse tree
    cout << "------------------------------CODE_GENERATION_C++------------------------------" << endl;
    generator.run_code(); //Print and execute program
}