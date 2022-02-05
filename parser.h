#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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
    token_tag[1] = token_tag[1].substr(0, token_tag[1].find('>'));
    cout << token_tag[0] << "    " << token_tag[1] << endl; num_token++; //to know in which part is and look if is an error
}
void add_error(string error, string* follows, int size_flls) {
    errors_parser.push_back(error);
    cout << error << endl;
    while (token_tag[0] != "EOF") {
        for (int i = 0; i < size_flls; i++) {
            if (token_tag[0] == *(follows + i) || token_tag[1] == *(follows + i)) return;
        }
        next_token();
    }
    return;
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
    //functions for each no-terminal
    void Program() { //Program → PROGRAM id /'(id)'|e/; ConstBlock VarBlock TypeBlock FunctionBlock ProcedureBlock MainCode
        //Follow is CONST,TYPE,VAR,FUNCTION,PROCEDURE,BEGIN,EOF
        string follows[] = { "CONST","VAR","TYPE","FUNCTION","PROCEDURE","BEGIN"};
        if (token_tag[0] != "PROGRAM") add_error("ErrorProgram: Find " + token_tag[1] + " but it must be 'program'.",follows,6);
        else {
            add_node(tree.root, "program"); next_token();
            if (token_tag[0] != "ID") return add_error("ErrorProgram: Find " + token_tag[1] + " but it must be 'id'.", follows, 6);
            add_node(tree.root, "id"); next_token();
            if (token_tag[0] == "DEL_OP") {
                add_node(tree.root, "("); next_token();
                if (token_tag[0] != "ID") return add_error("ErrorProgram: Find " + token_tag[1] + " but it must be 'id'.", follows, 6);
                add_node(tree.root, "id"); next_token();
                if (token_tag[0] != "DEL_CP") return add_error("ErrorProgram: Find " + token_tag[1] + " but it must be ')'.", follows, 6);
                add_node(tree.root, ")"); next_token();
            }
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorProgram: Find " + token_tag[1] + " but it must be ';'.", follows, 6);
            add_node(tree.root, ";"); next_token();
        }
        ConstBlock(tree.root); VarBlock(tree.root); TypeBlock(tree.root); FunctionBlock(tree.root); ProcedureBlock(tree.root);
        return MainCode(tree.root); //Obligatorio
    }
    void ConstBlock(PNode* dad) { //ConstBlock → CONST ConstList | Ɛ
        //Follow is TYPE,VAR,FUNCTION,PROCEDURE,BEGIN
        if (token_tag[0] != "CONST") return; //Ɛ
        add_node(dad, "const"); next_token();
        return ConstList(dad);
    }
    void ConstList(PNode* dad) { //ConstList → id = Value; ConstList | id = Value;
        add_node(dad, "ConstList"); //Follow is TYPE,VAR,FUNCTION,PROCEDURE,BEGIN
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "VAR","TYPE","FUNCTION","PROCEDURE","BEGIN"};
        if (token_tag[0] != "ID") return add_error("ErrorConstList: Find " + token_tag[1] + " but it must be 'id'.",follows,5);
        add_node(new_Dad, "id"); next_token();
        if (token_tag[0] != "OP_EQU") return add_error("ErrorConstList: Find " + token_tag[1] + " but it must be '='.", follows, 5);
        add_node(new_Dad, "="); next_token();
        Value(new_Dad); //already next_token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorConstList: Find " + token_tag[1] + " but it must be ';'.", follows, 5);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return ConstList(new_Dad); //Recursion
    }
    void Value(PNode* dad) { //Value → INTEGER | REAL | STRING 
        add_node(dad, "Value"); //Follow is ';',:,],),MUL,DIV,MOD,AND,+,-,OR,=,<>,<,<=,>=,>,',',OF,DO,THEN,break,continue,END,UNTIL,READ,READLN,WRITE,WRITELN,IF,ELSE,REPEAT,WHILE,FOR,CASE,GOTO,ID,TO,DOWNTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_COL","DEL_CBT","OP_MULT","OP_DIV","OP_DIV_ENT","OP_MOD","OP_AND","OP_SUM","OP_RES","OP_OR","OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DEL_COM",
        "OF","DO","THEN","DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "NUM_INT" && token_tag[0] != "NUM_REAL" && token_tag[0] != "STRING") 
            return add_error("ErrorValue: Find " + token_tag[1] + " but it must be 'v_int' or 'v_real' or 'v_string'.",follows,follows->size());
        add_node(new_Dad, "v_" + token_tag[0]); next_token();
    }
    void TypeBlock(PNode* dad) {//TypeBlock → TYPE TypeList | Ɛ
        //Follow is FUNCTION,PROCEDURE,BEGIN
        if (token_tag[0] != "TYPE") return; //Ɛ
        add_node(dad, "type"); next_token();
        return TypeList(dad);
    }
    void TypeList(PNode* dad) { //TypeList → NameList = TypeDecl ; TypeList | Ɛ
        add_node(dad, "TypeList"); //Follow is FUNCTION,PROCEDURE,BEGIN
        PNode* new_Dad = dad->nodes.back(); 
        string follows[] = { "FUNCTION","PROCEDURE","BEGIN" };
        NameList(new_Dad); //already next_token
        if (token_tag[0] != "OP_EQU") return add_error("ErrorTypeList: Find " + token_tag[1] + " but it must be '='.", follows, 3);
        add_node(new_Dad, "="); next_token();
        TypeDecl(new_Dad); //already next_token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorTypeList: Find " + token_tag[1] + " but it must be ';'.", follows, 3);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return TypeList(new_Dad); //Recursion
    }
    void NameList(PNode* dad) { //NameList → id | id, NameList
        add_node(dad, "NameList"); //Follow is =,:
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "OP_EQU" ,"DEL_COL"};
        if (token_tag[0] != "ID") return add_error("ErrorNameList: Find " + token_tag[1] + " but it must be 'id'.", follows, 2);
        add_node(new_Dad, "id"); next_token();
        if (token_tag[0] != "DEL_COM") return;
        add_node(new_Dad, ","); next_token();
        return NameList(new_Dad); //Recursion
    }
    void TypeDecl(PNode* dad) { //TypeDecl → SimpleDecl | ArrayDecl | RecordDecl
        add_node(dad, "TypeDecl"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC"};
        if (token_tag[1] == "integer" || token_tag[1] == "real" || token_tag[1] == "string" || token_tag[0] == "NUM_INT"
            || token_tag[0] == "STRING" || token_tag[0] == "OP_RES") return SimpleDecl(new_Dad);
        if (token_tag[0] == "ARRAY") return ArrayDecl(new_Dad);
        if (token_tag[0] == "RECORD") return RecordDecl(new_Dad);
        return add_error("ErrorTypeDecl: Find " + token_tag[1] + " but it must be 'int' or 'real' or 'string' or '-' or 'array' or 'record'.", follows, 1);;
    }
    void SimpleDecl(PNode* dad) { //SimpleDecl  → Value | INTEGER .. INTEGER | STRING .. STRING | - INTEGER .. INTEGER | -INTEGER .. - INTEGER
        add_node(dad, "SimpleDecl"); //Follow is ;,],',',)
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","DEL_COM","DEL_CBT","DEL_CP"};
        if (token_tag[1] == "real" || token_tag[1] == "integer" || token_tag[1] == "string") { add_node(new_Dad, token_tag[1]); next_token(); return; }
        if (token_tag[0] == "STRING" || token_tag[0] == "NUM_INT") { // INTEGER ..INTEGER | STRING ..STRING
            string last = token_tag[0];
            add_node(new_Dad, "v_" + last); next_token();
            if (token_tag[0] != "DEL_DD") return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be '..'.", follows, 4);
            add_node(new_Dad, ".."); next_token();
            if (token_tag[0] != last) return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be equal like the last.", follows, 4);
            add_node(new_Dad, "v_" + last); next_token();
            return;
        }
        if (token_tag[0] == "OP_RES") { //- INTEGER .. INTEGER | -INTEGER .. - INTEGER
            add_node(new_Dad, "-"); next_token();
            if (token_tag[0] != "NUM_INT") return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be 'int'.", follows, 4);
            add_node(new_Dad, "v_NUM_INT"); next_token();
            if (token_tag[0] != "DEL_DD") return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be '..'.", follows, 4);
            add_node(new_Dad, ".."); next_token();
            if (token_tag[0] == "NUM_INT") { add_node(new_Dad, "v_NUM_INT"); next_token(); return; }
            if (token_tag[0] == "OP_RES") {
                add_node(new_Dad, "-"); next_token();
                if (token_tag[0] != "NUM_INT") return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be 'int'.", follows, 4);
                add_node(new_Dad, "v_NUM_INT"); next_token(); return;
            }
            return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be 'int' or '-'.", follows, 4);;
        }
        return add_error("ErrorSimpleDecl: Find " + token_tag[1] + " but it must be 'int' or 'real' or 'string' or '-'.", follows, 4);;
    }
    void ArrayDecl(PNode* dad) { //ArrayDecl →  ARRAY[SimpleDecl] OF SimpleDecl
        add_node(dad, "ArrayDecl"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC" };
        if (token_tag[0] != "ARRAY") return add_error("ErrorArrayDecl: Find " + token_tag[1] + " but it must be 'array'.", follows, 1);
        add_node(new_Dad, "array"); next_token();
        if (token_tag[0] != "DEL_OB") return add_error("ErrorArrayDecl: Find " + token_tag[1] + " but it must be '['.", follows, 1);
        add_node(new_Dad, "["); next_token();
        SimpleDecl(new_Dad); //already next token
        if (token_tag[0] != "DEL_CBT") return add_error("ErrorArrayDecl: Find " + token_tag[1] + " but it must be ']'.", follows, 1);
        add_node(new_Dad, "]"); next_token();
        if (token_tag[0] != "OF") return add_error("ErrorArrayDecl: Find " + token_tag[1] + " but it must be 'of'.", follows, 1);
        add_node(new_Dad, "of"); next_token();
        SimpleDecl(new_Dad); //already next token
    }
    void RecordDecl(PNode* dad) { //RecordDecl → RECORD RecordList END;
        add_node(dad, "RecordDecl"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC" };
        if (token_tag[0] != "RECORD") return add_error("ErrorRecordDecl: Find " + token_tag[1] + " but it must be 'record'.", follows, 1);
        add_node(new_Dad, "record"); next_token();
        RecordList(new_Dad); //already next token
        if (token_tag[0] != "END") return add_error("ErrorRecordDecl: Find " + token_tag[1] + " but it must be 'end'.", follows, 1);
        add_node(new_Dad, "end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorRecordDecl: Find " + token_tag[1] + " but it must be ';'.", follows, 1);
        add_node(new_Dad, ";"); next_token(); 
    }
    void RecordList(PNode* dad) { //RecordList → NameList : TypeDecl; RecordList | Ɛ
        add_node(dad, "RecordList"); //Follow is END
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "END" };
        NameList(new_Dad); //already next token
        if (token_tag[0] != "DEL_COL") return add_error("ErrorRecordList: Find " + token_tag[1] + " but it must be ':'.", follows, 1);
        add_node(new_Dad, ":"); next_token();
        TypeDecl(new_Dad); //already next token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorRecordList: Find " + token_tag[1] + " but it must be ';'.", follows, 1);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return RecordList(new_Dad); //Recursion
    }
    void VarBlock(PNode* dad) { //VarBlock → VAR VarList | Ɛ
        //Follow is TYPE,FUNCTION,PROCEDURE,BEGIN
        if (token_tag[0] != "VAR") return; //Ɛ
        add_node(dad, "VarBlock");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad, "var"); next_token();
        return VarList(new_Dad);
    }
    void VarList(PNode* dad) { //VarList → NameList : TypeDecl; VarList | Ɛ
        add_node(dad, "VarList"); //Follow is TYPE,FUNCTION,PROCEDURE,BEGIN
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "TYPE","FUNCTION","PROCEDURE","BEGIN" };
        NameList(new_Dad);
        if (token_tag[0] != "DEL_COL") return add_error("ErrorVarList: Find " + token_tag[1] + " but it must be ':'.", follows, 4);
        add_node(new_Dad, ":"); next_token();
        TypeDecl(new_Dad); //already next token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorVarList: Find " + token_tag[1] + " but it must be ';'.", follows, 4);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return VarList(new_Dad); //Recursion
    }
    void FunctionBlock(PNode* dad) { //FunctionBlock → FUNCTION id Parameters : SimpleDecl; BEGIN StmtList END; | e
        //Follow is PROCEDURE,BEGIN
        if (token_tag[0] != "FUNCTION") return; //Ɛ
        add_node(dad, "FunctionBlock");
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "PROCEDURE","BEGIN" };
        add_node(new_Dad, "function"); next_token();
        if (token_tag[0] != "ID") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be 'id'.", follows, 2); //Ɛ
        add_node(new_Dad, "id"); next_token();
        Parameters(new_Dad); //already next token
        if (token_tag[0] != "DEL_COL") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be ':'.", follows, 2);
        add_node(new_Dad, ":"); next_token();
        SimpleDecl(new_Dad); //already next token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be ';'.", follows, 2);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] != "BEGIN") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be 'begin'.", follows, 2);
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad); //already next token
        if (token_tag[0] != "END") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be 'end'.", follows, 2);
        add_node(new_Dad, "end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorFunctionBlock: Find " + token_tag[1] + " but it must be ';'.", follows, 2);
        add_node(new_Dad, ";"); next_token();
    }
    void ProcedureBlock(PNode* dad) { //ProcedureBlock → PROCEDURE id Parameters; BEGIN StmtList END; | e
        //Follow is BEGIN
        if (token_tag[0] != "PROCEDURE") return; //Ɛ
        add_node(dad, "ProcedureBlock");
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "BEGIN"};
        add_node(new_Dad, "procedure"); next_token();
        if (token_tag[0] != "ID") return add_error("ErrorProcedureBlock: Find " + token_tag[1] + " but it must be 'id'.", follows, 1); //Ɛ
        add_node(new_Dad, "id"); next_token();
        Parameters(new_Dad); //already next token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorProcedureBlock: Find " + token_tag[1] + " but it must be ';'.", follows, 1);
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] != "BEGIN") return add_error("ErrorProcedureBlock: Find " + token_tag[1] + " but it must be 'begin'.", follows, 1);
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad); //already next token
        if (token_tag[0] != "END") return add_error("ErrorProcedureBlock: Find " + token_tag[1] + " but it must be 'end'.", follows, 1);
        add_node(new_Dad, "end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorProcedureBlock: Find " + token_tag[1] + " but it must be ';'.", follows, 1);
        add_node(new_Dad, ";"); next_token();
    }
    void Parameters(PNode* dad) { //Parameters →() | (ParamList)
        add_node(dad, "Parameters"); //Follow is :,;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","DEL_COL"};
        if (token_tag[0] != "DEL_OP") return add_error("ErrorParameters: Find " + token_tag[1] + " but it must be '('.", follows, 2);
        add_node(new_Dad, "("); next_token();
        if (token_tag[0] == "VAR" || token_tag[0] == "ID") { //e
            ParamList(new_Dad); //already next token
        }
        if (token_tag[0] != "DEL_CP") return add_error("ErrorParameters: Find " + token_tag[1] + " but it must be ')'.", follows, 2);
        add_node(new_Dad, ")"); next_token();
    }
    void ParamList(PNode* dad) { //ParamList → VAR NameList : SimpleDecl ParamCont | NameList : SimpleDecl ParamCont ....... //ParamCont →, ParamList | e
        add_node(dad, "ParamList"); //Follow is )
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_CP"};
        if (token_tag[0] == "VAR") { add_node(new_Dad, "var"); next_token(); } //var or not
        NameList(new_Dad); //already next token
        if (token_tag[0] != "DEL_COL") return add_error("ErrorParamList: Find " + token_tag[1] + " but it must be ':'.", follows, 1);
        add_node(new_Dad, ":"); next_token();
        SimpleDecl(new_Dad); //already next token
        if (token_tag[0] != "DEL_COM") return; //Ɛ
        add_node(new_Dad, ","); next_token();
        return ParamList(new_Dad); //Recursion
    }
    void StmtList(PNode* dad) { //StmtList → Stmt StmtList | e
        add_node(dad, "StmtList"); //Follow is END,UNTIL
        PNode* new_Dad = dad->nodes.back();
        Stmt(new_Dad);
        if (token_tag[0] == "ID" || token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln" ||
            token_tag[0] == "IF" || token_tag[0] == "ELSE" || token_tag[0] == "REPEAT" || token_tag[0] == "WHILE" || token_tag[0] == "FOR" ||
            token_tag[0] == "CASE" || token_tag[0] == "GOTO") return StmtList(new_Dad); //Recursion
    }
    void Stmt(PNode* dad) { //Stmt → break; | continue; | Assign; | Proc; | IfBlock | ElseBolck | RepeatBlock; | WhileBlock | ForBlock | CaseBlock | GotoBlock;    //GotoBlock → GOTO INTEGER
        add_node(dad, "Stmt"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[1] == "break" || token_tag[1] == "continue") {
            add_node(new_Dad, token_tag[1]); next_token();
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        if (token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln") {
            Proc(new_Dad); //already next token
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        if (token_tag[0] == "ID") {
            Assign(new_Dad); //already next token
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        if (token_tag[0] == "IF") return IfBlock(new_Dad);
        if (token_tag[0] == "ELSE") return ElseBlock(new_Dad);
        if (token_tag[0] == "REPEAT") { 
            RepeatBlock(new_Dad); //already next token
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        if (token_tag[0] == "WHILE") return WhileBlock(new_Dad);
        if (token_tag[0] == "FOR") return ForBlock(new_Dad);
        if (token_tag[0] == "CASE") return CaseBlock(new_Dad);
        if (token_tag[0] == "GOTO") {  //GotoBlock;  //GotoBlock → GOTO INTEGER
            add_node(new_Dad, "GotoBlock");
            new_Dad = new_Dad->nodes.back();
            add_node(new_Dad, "goto"); next_token();
            if (token_tag[0] != "NUM_INT") return add_error("ErrorGotoBlock: Find " + token_tag[1] + " but it must be 'int'.", follows, follows->size());
            add_node(new_Dad, "v_NUM_INT"); next_token();
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token(); return;
        }
        return add_error("ErrorStmt: Find " + token_tag[1] + " but it must be 'id' or 'read' or 'readln' or 'write' or 'writeln or 'if' or 'else' or 'repeat' or 'while' or 'for' or 'case' or 'goto'.", follows, follows->size());
    }
    void Assign(PNode* dad) { //Assign → ID := Expr
        add_node(dad, "Assign"); //Follow is TO,DOWNTO,;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC" ,"TO","DOWNTO"};
        if (token_tag[0] != "ID") return add_error("ErrorAssign: Find " + token_tag[1] + " but it must be 'id'.", follows, follows->size());
        add_node(new_Dad, "id"); next_token();
        if (token_tag[0] != "OP_ASIG") return add_error("ErrorAssign: Find " + token_tag[1] + " but it must be ':='.", follows, follows->size());
        add_node(new_Dad, ":="); next_token();
        return Expr(new_Dad);
    }
    void Proc(PNode* dad) { //Proc → READ args | READLN args | WRITE args | WRITELN args
        add_node(dad, "Proc"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC" };
        if (token_tag[1] != "read" && token_tag[1] != "readln" && token_tag[1] != "write" && token_tag[1] != "writeln")
            return add_error("ErrorProc: Find " + token_tag[1] + " but it must be 'read' or 'readln' or 'write' or 'writeln'.", follows, 1);
        add_node(new_Dad, token_tag[1]); next_token();
        return args(new_Dad);
    }
    void args(PNode* dad) { //args → (ExprList)
        add_node(dad, "Args"); //Follow is ;
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC"};
        if (token_tag[0] != "DEL_OP") return add_error("ErrorArgs: Find " + token_tag[1] + " but it must be '('.", follows,1 );
        add_node(new_Dad, "("); next_token();
        ExprList(new_Dad);
        if (token_tag[0] != "DEL_CP") return add_error("ErrorArgs: Find " + token_tag[1] + " but it must be ')'.", follows, 1);
        add_node(new_Dad, ")"); next_token();
    }
    void IfBlock(PNode* dad) { //IfBlock → IF Expr THEN Stmt ElseBlock
        add_node(dad, "IfBlock"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "IF") return add_error("ErrorIfBlock: Find " + token_tag[1] + " but it must be 'if'.", follows, follows->size());
        add_node(new_Dad, "if"); next_token();
        Expr(new_Dad);
        if (token_tag[0] != "THEN") return add_error("ErrorIfBlock: Find " + token_tag[1] + " but it must be 'then'.", follows, follows->size());
        add_node(new_Dad, "then"); next_token();
        Stmt(new_Dad);
        return ElseBlock(new_Dad);
    }
    void ElseBlock(PNode* dad) { //ElseBolck → ELSE Stmt | e
        //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        if (token_tag[0] != "ELSE") return; //Ɛ
        add_node(dad, "ElseBolck");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad, "else"); next_token();
        return Stmt(new_Dad);
    }
    void RepeatBlock(PNode* dad) { //RepeatBlock → REPEAT StmtList UNTIL Expr
        add_node(dad, "RepeatBlock"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "REPEAT") return add_error("ErrorRepeatBlock: Find " + token_tag[1] + " but it must be 'repeat'.", follows, follows->size());
        add_node(new_Dad, "repeat"); next_token();
        StmtList(new_Dad);
        if (token_tag[0] != "UNTIL") return add_error("ErrorRepeatBlock: Find " + token_tag[1] + " but it must be 'until'.", follows, follows->size());
        add_node(new_Dad, "until"); next_token();
        return Expr(new_Dad);
    }
    void WhileBlock(PNode* dad) { //WhileBlock → WHILE Expr DO BEGIN|e StmtList END|e;
        add_node(dad, "WhileBlock"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "WHILE") return add_error("ErrorWhileBlock: Find " + token_tag[1] + " but it must be 'while'.", follows, follows->size());
        add_node(new_Dad, "while"); next_token();
        Expr(new_Dad);
        if (token_tag[0] != "DO") return add_error("ErrorWhileBlock: Find " + token_tag[1] + " but it must be 'do'.", follows, follows->size());
        add_node(new_Dad, "do"); next_token();
        bool is_begin = 0;
        if (token_tag[0] == "BEGIN") add_node(new_Dad, "begin"), next_token(), is_begin = 1; //e
        StmtList(new_Dad);
        if (token_tag[0] == "END" && is_begin) { //e
            add_node(new_Dad, "end"), next_token();
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorWhileBlock: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token();
        }
    }
    void ForBlock(PNode* dad) { //ForBlock → FOR Assign Direction Expr DO BEGIN|e StmtList END|e; ..... //Direction → TO | DOWNTO
        add_node(dad, "ForBlock"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "FOR") return add_error("ErrorForBlock: Find " + token_tag[1] + " but it must be 'for'.", follows, follows->size());
        add_node(new_Dad, "for"); next_token();
        Assign(new_Dad);
        if (token_tag[0] != "TO" && token_tag[0] != "DOWNTO") return add_error("ErrorForBlock: Find " + token_tag[1] + " but it must be 'to' or 'downto'.", follows, follows->size());
        add_node(new_Dad, token_tag[0]); next_token();
        Expr(new_Dad);
        if (token_tag[0] != "DO") return add_error("ErrorForBlock: Find " + token_tag[1] + " but it must be 'do'.", follows, follows->size());
        add_node(new_Dad, "do"); next_token();
        bool is_begin = 0;
        if (token_tag[0] == "BEGIN") add_node(new_Dad, "begin"), next_token(), is_begin = 1; //e
        StmtList(new_Dad);
        if (token_tag[0] == "END" && is_begin) { //e
            add_node(new_Dad, "end"), next_token();
            if (token_tag[0] != "DEL_SEMC") return add_error("ErrorForBlock: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
            add_node(new_Dad, ";"); next_token();
        }
    }
    void CaseBlock(PNode* dad) { //CaseBlock → CASE Expr OF CaseList END
        add_node(dad, "CaseBlock"); //Follow is ;,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] != "CASE") return add_error("ErrorCaseBlock: Find " + token_tag[1] + " but it must be 'case'.", follows, follows->size());
        add_node(new_Dad, "case"); next_token();
        Expr(new_Dad);
        if (token_tag[0] != "OF") return add_error("ErrorCaseBlock: Find " + token_tag[1] + " but it must be 'of'.", follows, follows->size());
        add_node(new_Dad, "of"); next_token();
        CaseList(new_Dad);
        if (token_tag[0] != "END") return add_error("ErrorCaseBlock: Find " + token_tag[1] + " but it must be 'end'.", follows, follows->size());
        add_node(new_Dad, "end"); next_token();
    }
    void CaseList(PNode* dad) { //CaseList → CaseExpr CaseList | e
        add_node(dad, "CaseList"); //Follow is END
        PNode* new_Dad = dad->nodes.back();
        CaseExpr(new_Dad);
        if (token_tag[0] == "NUM_INT" || token_tag[0] == "NUM_REAL" || token_tag[0] == "STRING" || token_tag[0] == "ID")
            return CaseList(new_Dad); //Ɛ
    }
    void CaseExpr(PNode* dad) { //CaseExpr → Value : Stmt; | ID : Stmt;
        add_node(dad, "CaseExpr"); //Follow is END,ID,NUM_INT,NUM_REAL,STRING
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "ID","NUM_INT","NUM_REAL","STRING","END" };
        if (token_tag[0] != "NUM_INT" && token_tag[0] != "NUM_REAL" && token_tag[0] != "STRING" && token_tag[0] != "ID")
            return add_error("ErrorCaseExpr: Find " + token_tag[1] + " but it must be 'int' or 'real' or 'string' or 'id'.", follows, follows->size());
        add_node(new_Dad, token_tag[0]); next_token();
        if (token_tag[0] != "DEL_COL") return add_error("ErrorCaseExpr: Find " + token_tag[1] + " but it must be ':'.", follows, follows->size());
        add_node(new_Dad, ":"); next_token();
        Stmt(new_Dad); //already next token
        if (token_tag[0] != "DEL_SEMC") return add_error("ErrorCaseExpr: Find " + token_tag[1] + " but it must be ';'.", follows, follows->size());
        add_node(new_Dad, ";"); next_token();
    }
    void ExprList(PNode* dad) { //ExprList → Expr, ExprList | e
        add_node(dad, "ExprList"); //Follow is )
        PNode* new_Dad = dad->nodes.back();
        Expr(new_Dad);
        if (token_tag[0] != "DEL_COM") return; //e
        add_node(new_Dad, ","); next_token();
        return ExprList(new_Dad); //Recursion
    }
    void Expr(PNode* dad) { //Expr → Expr_ RelOp Expr | Expr_
        add_node(dad, "Expr"); //Follow is ',',OF,DO,THEN,';',TO,DOWNTO,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        Expr_(new_Dad);
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return;
        RelOp(new_Dad);
        return Expr(new_Dad); //Recursion
    }
    void RelOp(PNode* dad) { //RelOp → = | <> | < | <= | >= | >
        add_node(dad, "RelOp"); //Follow is ID,NUM_INT,NUM_REAL,STRING,FUNCTION,(,NOT,MINUS
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "ID","NUM_INT","NUM_REAL","STRING","FUNCTION","DEL_OP","OP_NOT","OP_RES"};
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return add_error("ErrorRelOP: Find " + token_tag[1] + " but it must be '=' or '<>' or '<' or '<=' or '>=' or '>'.",follows,follows->size());
        add_node(new_Dad, token_tag[0]); next_token();
    }
    void Expr_(PNode* dad) { //Expr_ → Term + Expr_ | Term - Expr_ | Term OR Expr_ | Term
        add_node(dad, "Expr_"); //Follow is =,<>,<,<=,>=,>,',',OF,DO,THEN,';',TO,DOWNTO,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        Term(new_Dad);
        if (token_tag[0] != "OP_SUM" && token_tag[0] != "OP_RES" && token_tag[0] != "OP_OR")
            return; //Term
        add_node(new_Dad, token_tag[0]); next_token();
        return Expr_(new_Dad); //Recursion
    }
    void Term(PNode* dad) { //Term → Factor MUL|DIV|MOD|AND Term    |    Factor
        add_node(dad, "Term"); //Follow is +,-,OR,=,<>,<,<=,>=,>,',',OF,DO,THEN,';',TO,DOWNTO,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        Factor(new_Dad);
        if (token_tag[0] != "OP_MULT" && token_tag[0] != "OP_DIV" && token_tag[0] != "OP_DIV_ENT" && token_tag[0] != "OP_MOD" && token_tag[0] != "OP_AND")
            return; //Factor
        add_node(new_Dad, token_tag[0]); next_token();
        return Term(new_Dad); //Recursion
    }
    void Factor(PNode* dad) { //Factor → ID | Value | FUNCTION(ExprList) | (ExprList) | NOT Factor | MINUS Factor
        add_node(dad, "Factor"); //Follow is MUL,DIV,MOD,AND,+,-,OR,=,<>,<,<=,>=,>,',',OF,DO,THEN,';',TO,DOWNTO,ELSE,END,UNTIL,BREAK,CONTINUE,ID,READ,READLN,WRITE,WRITELN,IF,REPEAT,WHILE,FOR,CASE,GOTO
        PNode* new_Dad = dad->nodes.back();
        string follows[] = { "OP_MULT","OP_DIV","OP_DIV_ENT","OP_MOD","OP_AND","OP_SUM","OP_RES","OP_OR","OP_EQU","OP_NOT_EQU","OP_LT","OP_LE","OP_GE","OP_GT","DEL_COM",
        "OF","DO","THEN","DEL_SEMC","TO","DOWNTO","ELSE","END","UNTIL","ID","IF","REPEAT","WHILE","FOR","CASE","GOTO","break","continue","read","readln","write","writeln" };
        if (token_tag[0] == "FUNCTION") { //FUNCTION(ExprList) 
            add_node(new_Dad, "function"); next_token();
            if (token_tag[0] != "DEL_OP") return add_error("ErrorFactor: Find " + token_tag[1] + " but it must be '('.", follows, follows->size());
            add_node(new_Dad, "("); next_token();
            ExprList(new_Dad);
            if (token_tag[0] != "DEL_CP") return add_error("ErrorFactor: Find " + token_tag[1] + " but it must be ')'.", follows, follows->size());
            add_node(new_Dad, ")"); next_token();
            return;
        }
        if (token_tag[0] == "ID" || token_tag[0] == "NUM_INT" || token_tag[0] == "NUM_REAL" || token_tag[0] == "STRING") { //ID | Value
            add_node(new_Dad, "v_" + token_tag[0]); next_token();
            return;
        }
        if (token_tag[0] == "DEL_OP") { //(ExprList)
            add_node(new_Dad, "("); next_token();
            ExprList(new_Dad);
            if (token_tag[0] != "DEL_CP") return add_error("ErrorFactor: Find " + token_tag[1] + " but it must be ')'.", follows, follows->size());
            add_node(new_Dad, ")"); next_token();
            return;
        }
        if (token_tag[0] == "OP_NOT" || token_tag[0] == "OP_RES") { //NOT Factor  //MINUS Factor
            add_node(new_Dad, token_tag[0]); next_token();
            return Factor(new_Dad);
        }
        return add_error("ErrorFactor: Find " + token_tag[1] + " but it must be 'id' or 'int' or 'real' or 'string' or 'function' or '(' or 'not' or '-'.", follows, follows->size());
    }
    void MainCode(PNode* dad) { //MainCode → BEGIN StmtList END.
        add_node(dad, "MainCode"); //Follow is EOF $
        PNode* new_Dad = dad->nodes.back(); string follows[] = { "EOF" };
        if (token_tag[0] != "BEGIN") return add_error("ErrorMainCode: Find " + token_tag[1] + " but it must be 'begin'.", follows, 1);
        add_node(new_Dad, "begin"); next_token();
        StmtList(new_Dad);
        if (token_tag[0] != "END") return add_error("ErrorMainCode: Find " + token_tag[1] + " but it must be 'end'.", follows, 1);
        add_node(new_Dad, "end"); next_token();
        if (token_tag[0] != "OP_DOT") return add_error("ErrorMainCode: Find " + token_tag[1] + " but it must be '.'.", follows, 1);
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
    cout << "------------------------------PARSE_TREE------------------------------" << endl;
    print_parse_tree(tree.root); //Print parse tree
}