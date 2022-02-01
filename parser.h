#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

int num_token = 0;
string token_tag[2]; //token to see
vector<string> tags;

void get_vector(vector<string> vec) { //copy tokens
    for (int i = 0; i < vec.size(); i++)
        tags.push_back(vec[i]); //token
}
void next_token() { //next token to see the production of grammar
    token_tag[0] = tags[num_token].substr(1, tags[num_token].find(',') - 1); //tag
    token_tag[1] = tags[num_token].substr(tags[num_token].find(',') + 1); //value
    token_tag[1] = token_tag[1].substr(0, token_tag[1].find('>'));
    cout << token_tag[0] <<"    "<< token_tag[1] << endl; num_token++; //to know in which part is and look if is an error
}
bool print_error(string error) {
    cout << error << endl;
    return false;
}
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
void add_node(PNode* dad, string child) { //create token_node in the tree
    tree.insert(dad, child);
}
void parse_tree(PNode* node) { //Print recursively parse tree 
    if (node->nodes.empty()) return; //is terminal --> return, else continue
    //cout << node->value << "    ->  ";
    int s = node->nodes.size(), temp = node->space; //print spaces of dad and mine
    while (temp > 0) cout << ' ', temp--;
    cout << "|"; //to know which nonterminal it corresponds to 
    for (temp = 0; temp < s; temp++) //print children
        cout << node->nodes[temp]->value << ' ';
    cout << endl;
    for (temp = 0; temp < s; temp++) //function to children
        parse_tree(node->nodes[temp]);
}
//Struct parser
struct parser_class {
    //functions for each no-terminal
    bool Program() { //Program → PROGRAM id /'(id)'|e/; ConstBlock VarBlock TypeBlock FunctionBlock ProcedureBlock MainCode
        if (token_tag[0] != "PROGRAM") return print_error("ErrorProgram: Supposed to be 'program'.");
        add_node(tree.root,"program"); next_token();
        if (token_tag[0] != "ID") return print_error("ErrorProgram: Supposed to be 'id'.");
        add_node(tree.root, "id"); next_token();
        if (token_tag[0] == "DEL_OP") {
            add_node(tree.root, "("); next_token();
            if (token_tag[0] != "ID") return print_error("ErrorProgram: Supposed to be 'id'.");
            add_node(tree.root, "id"); next_token();
            if (token_tag[0] != "DEL_CP") return print_error("ErrorProgram: Supposed to be ')'.");
            add_node(tree.root, ")"); next_token();
        }
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorProgram: Supposed to be ';'.");
        add_node(tree.root, ";"); next_token();
        if (!ConstBlock(tree.root) || !VarBlock(tree.root) || !TypeBlock(tree.root) || !FunctionBlock(tree.root) || !ProcedureBlock(tree.root)) 
            return false; //Alguno no cumple 
        return MainCode(tree.root); //Obligatorio
    }
    bool ConstBlock(PNode* dad) { //ConstBlock → CONST ConstList | Ɛ
        if (token_tag[0] != "CONST") return true; //Ɛ
        add_node(dad, "const"); next_token();
        return ConstList(dad);
    }
    bool ConstList(PNode* dad) { //ConstList → id = Value; ConstList | id = Value;
        add_node(dad, "ConstList");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "ID") return print_error("ErrorConstList: Supposed to be 'id'.");
        add_node(new_Dad,"id"); next_token();
        if (token_tag[0] != "OP_EQU") return print_error("ErrorConstList: Supposed to be '='.");
        add_node(new_Dad, "="); next_token();
        if (!Value(new_Dad)) return false; //already next_token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorConstList: Supposed to be ';'.");
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return ConstList(new_Dad); //Recursion
        return true;
    }
    bool Value(PNode* dad) { //Value → INTEGER | REAL | STRING 
        add_node(dad, "Value");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "NUM_INT" && token_tag[0] != "NUM_REAL" && token_tag[0] != "STRING") return print_error("ErrorValue: Supposed to be 'v_int' or 'v_real' or 'v_string'.");
        add_node(new_Dad, "v_" + token_tag[0]); next_token();
        return true;
    }
    bool TypeBlock(PNode* dad) {//TypeBlock → TYPE TypeList | Ɛ
        if (token_tag[0] != "TYPE") return true; //Ɛ
        add_node(dad, "type"); next_token();
        return TypeList(dad);
    }
    bool TypeList(PNode* dad) { //TypeList → NameList = TypeDecl ; TypeList | Ɛ
        add_node(dad, "TypeList");
        PNode* new_Dad = dad->nodes.back();
        if (!NameList(new_Dad)) return false; //already next_token
        if (token_tag[0] != "OP_EQU") return print_error("ErrorTypeList: Supposed to be '='.");
        add_node(new_Dad, "="); next_token();
        if (!TypeDecl(new_Dad)) return false; //already next_token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorTypeList: Supposed to be ';'.");
        add_node(new_Dad, ";"); next_token();
        if (token_tag[0] == "ID") return TypeList(new_Dad); //Recursion
        return true; //e
    }
    bool NameList(PNode* dad) { //NameList → id | id, NameList
        add_node(dad, "NameList");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "ID") return print_error("ErrorNameList: Supposed to be 'id'.");
        add_node(new_Dad, "id"); next_token();
        if (token_tag[0] != "DEL_COM") return true;
        add_node(new_Dad,","); next_token();
        return NameList(new_Dad); //Recursion
    }
    bool TypeDecl(PNode* dad) { //TypeDecl → SimpleDecl | ArrayDecl | RecordDecl
        add_node(dad, "TypeDecl");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[1] == "integer" || token_tag[1] == "real" || token_tag[1] == "string" || token_tag[0] == "NUM_INT" 
            || token_tag[0] == "STRING" || token_tag[0] == "OP_RES") return SimpleDecl(new_Dad);
        if (token_tag[0] == "ARRAY") return ArrayDecl(new_Dad);
        if (token_tag[0] == "RECORD") return RecordDecl(new_Dad);
        return print_error("ErrorTypeDecl: Supposed to be 'int' or 'real' or 'string' or '-' or 'array' or 'record'.");;
    }
    bool SimpleDecl(PNode* dad) { //SimpleDecl  → Value | INTEGER .. INTEGER | STRING .. STRING | - INTEGER .. INTEGER | -INTEGER .. - INTEGER
        add_node(dad, "SimpleDecl");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[1] == "real" || token_tag[1] == "integer" || token_tag[1] == "string") { add_node(new_Dad, token_tag[1]); next_token(); return true; }
        if (token_tag[0] == "STRING" || token_tag[0] == "NUM_INT") { // INTEGER ..INTEGER | STRING ..STRING
            string last = token_tag[0];
            add_node(new_Dad, "v_" + last); next_token();
            if (token_tag[0] != "DEL_DD") return print_error("ErrorSimpleDecl: Supposed to be '..'.");
            add_node(new_Dad, ".."); next_token();
            if (token_tag[0] != last) return print_error("ErrorSimpleDecl: Supposed to be equal like the last.");
            add_node(new_Dad, "v_" + last); next_token();
            return true;
        }
        if (token_tag[0] == "OP_RES") { //- INTEGER .. INTEGER | -INTEGER .. - INTEGER
            add_node(new_Dad, "-"); next_token();
            if (token_tag[0] != "NUM_INT") return print_error("ErrorSimpleDecl: Supposed to be 'int'.");
            add_node(new_Dad, "v_NUM_INT"); next_token();
            if (token_tag[0] != "DEL_DD") return print_error("ErrorSimpleDecl: Supposed to be '..'.");
            add_node(new_Dad, ".."); next_token();
            if (token_tag[0] == "NUM_INT") { add_node(new_Dad, "v_NUM_INT"); next_token(); return true; }
            if (token_tag[0] == "OP_RES") {
                add_node(new_Dad, "-"); next_token();
                if (token_tag[0] != "NUM_INT") return print_error("ErrorSimpleDecl: Supposed to be 'int'.");
                add_node(new_Dad, "v_NUM_INT"); next_token(); return true;
            }
            return print_error("ErrorSimpleDecl: Supposed to be 'int' or '-'.");;
        }
        return print_error("ErrorSimpleDecl: Supposed to be 'int' or 'real' or 'string' or '-'.");;
    }
    bool ArrayDecl(PNode* dad) { //ArrayDecl →  ARRAY[SimpleDecl] OF SimpleDecl
        add_node(dad, "ArrayDecl");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "ARRAY") return print_error("ErrorArrayDecl: Supposed to be 'array'.");
        add_node(new_Dad, "array"); next_token();
        if (token_tag[0] != "DEL_OB") return print_error("ErrorArrayDecl: Supposed to be '['.");
        add_node(new_Dad, "["); next_token();
        if (!SimpleDecl(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_CBT") return print_error("ErrorArrayDecl: Supposed to be ']'.");
        add_node(new_Dad, "]"); next_token();
        if (token_tag[0] != "OF") return print_error("ErrorArrayDecl: Supposed to be 'of'.");
        add_node(new_Dad, "of"); next_token();
        if (!SimpleDecl(new_Dad)) return false; //already next token
        return true;
    }
    bool RecordDecl(PNode* dad) { //RecordDecl → RECORD RecordList END;
        add_node(dad, "RecordDecl");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "RECORD") return print_error("ErrorRecordDecl: Supposed to be 'record'.");
        add_node(new_Dad,"record"); next_token();
        if (!RecordList(new_Dad)) return false; //already next token
        if (token_tag[0] != "END") return print_error("ErrorRecordDecl: Supposed to be 'end'.");
        add_node(new_Dad,"end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorRecordDecl: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token(); return true;
    }
    bool RecordList(PNode* dad) { //RecordList → NameList : TypeDecl; RecordList | Ɛ
        add_node(dad, "RecordList");
        PNode* new_Dad = dad->nodes.back();
        if (!NameList(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_COL") return print_error("ErrorRecordList: Supposed to be ':'.");
        add_node(new_Dad,":"); next_token();
        if (!TypeDecl(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorRecordList: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        if (token_tag[0] == "ID") return RecordList(new_Dad); //Recursion
        return true; //e
    }
    bool VarBlock(PNode* dad) { //VarBlock → VAR VarList | Ɛ
        if (token_tag[0] != "VAR") return true; //Ɛ
        add_node(dad, "VarBlock");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad,"var"); next_token();
        return VarList(new_Dad);
    }
    bool VarList(PNode* dad) { //VarList → NameList : TypeDecl; VarList | Ɛ
        add_node(dad, "VarList");
        PNode* new_Dad = dad->nodes.back();
        if (!NameList(new_Dad)) return false;
        if (token_tag[0] != "DEL_COL") return print_error("ErrorVarList: Supposed to be ':'.");
        add_node(new_Dad,":"); next_token();
        if (!TypeDecl(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorVarList: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        if (token_tag[0] == "ID") return VarList(new_Dad); //Recursion
        return true; //e
    }
    bool FunctionBlock(PNode* dad) { //FunctionBlock → FUNCTION id Parameters : SimpleDecl; BEGIN StmtList END; | e
        if (token_tag[0] != "FUNCTION") return true; //Ɛ
        add_node(dad, "FunctionBlock");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad,"function"); next_token();
        if (token_tag[0] != "ID") return print_error("ErrorFunctionBlock: Supposed to be 'id'."); //Ɛ
        add_node(new_Dad,"id"); next_token();
        if (!Parameters(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_COL") return print_error("ErrorFunctionBlock: Supposed to be ':'.");
        add_node(new_Dad,":"); next_token();
        if (!SimpleDecl(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorFunctionBlock: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        if (token_tag[0] != "BEGIN") return print_error("ErrorFunctionBlock: Supposed to be 'begin'.");
        add_node(new_Dad,"begin"); next_token();
        if (!StmtList(new_Dad)) return false; //already next token
        if (token_tag[0] != "END") return print_error("ErrorFunctionBlock: Supposed to be 'end'.");
        add_node(new_Dad,"end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorFunctionBlock: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        return true;
    }
    bool ProcedureBlock(PNode* dad) { //ProcedureBlock → PROCEDURE id Parameters; BEGIN StmtList END; | e
        if (token_tag[0] != "PROCEDURE") return true; //Ɛ
        add_node(dad, "ProcedureBlock");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad,"procedure"); next_token();
        if (token_tag[0] != "ID") return print_error("ErrorProcedureBlock: Supposed to be 'id'."); //Ɛ
        add_node(new_Dad,"id"); next_token();
        if (!Parameters(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorProcedureBlock: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        if (token_tag[0] != "BEGIN") return print_error("ErrorProcedureBlock: Supposed to be 'begin'.");
        add_node(new_Dad,"begin"); next_token();
        if (!StmtList(new_Dad)) return false; //already next token
        if (token_tag[0] != "END") return print_error("ErrorProcedureBlock: Supposed to be 'end'.");
        add_node(new_Dad,"end"); next_token();
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorProcedureBlock: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        return true;
    }
    bool Parameters(PNode* dad) { //Parameters →() | (ParamList)
        add_node(dad, "Parameters");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "DEL_OP") return print_error("ErrorParameters: Supposed to be '('.");
        add_node(new_Dad,"("); next_token();
        if (token_tag[0] == "VAR" || token_tag[0] == "ID") { //e
            if (!ParamList(new_Dad)) return false; //already next token
        }
        if (token_tag[0] != "DEL_CP") return print_error("ErrorParameters: Supposed to be ')'.");
        add_node(new_Dad,")"); next_token();
        return true;
    }
    bool ParamList(PNode* dad) { //ParamList → VAR NameList : SimpleDecl ParamCont | NameList : SimpleDecl ParamCont ....... //ParamCont →, ParamList | e
        add_node(dad, "ParamList");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] == "VAR") { add_node(new_Dad,"var"); next_token(); } //var or not
        if (!NameList(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_COL") return print_error("ErrorParamList: Supposed to be ':'.");
        add_node(new_Dad,":"); next_token();
        if (!SimpleDecl(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_COM") return true; //Ɛ
        add_node(new_Dad,","); next_token();
        return ParamList(new_Dad); //Recursion
    }
    bool StmtList(PNode* dad) { //StmtList → Stmt StmtList | e
        add_node(dad, "StmtList");
        PNode* new_Dad = dad->nodes.back();
        if (!Stmt(new_Dad)) return false;
        if (token_tag[0] == "ID" || token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln" ||
            token_tag[0] == "IF" || token_tag[0] == "ELSE" || token_tag[0] == "REPEAT" || token_tag[0] == "WHILE" || token_tag[0] == "FOR" ||
            token_tag[0] == "CASE" || token_tag[0] == "GOTO") return StmtList(new_Dad); //Recursion
        return true; //Ɛ
    }
    bool Stmt(PNode* dad) { //Stmt → break; | continue; | Assign; | Proc; | IfBlock | ElseBolck | RepeatBlock | WhileBlock | ForBlock | CaseBlock | GotoBlock;    //GotoBlock → GOTO INTEGER
        add_node(dad, "Stmt");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[1] == "break" || token_tag[1] == "continue") {
            add_node(new_Dad,token_tag[1]); next_token();
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorStmt: Supposed to be ';'.");
            add_node(new_Dad,";"); next_token(); return true;
        }
        if (token_tag[1] == "read" || token_tag[1] == "readln" || token_tag[1] == "write" || token_tag[1] == "writeln") {
            if (!Proc(new_Dad)) return false; //already next token
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorStmt: Supposed to be ';'.");
            add_node(new_Dad,";"); next_token(); return true;
        }
        if (token_tag[0] == "ID") {
            if (!Assign(new_Dad)) return false; //already next token
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorStmt: Supposed to be ';'.");
            add_node(new_Dad,";"); next_token(); return true;
        }
        if (token_tag[0] == "IF") return IfBlock(new_Dad);
        if (token_tag[0] == "ELSE") return ElseBlock(new_Dad);
        if (token_tag[0] == "REPEAT") return RepeatBlock(new_Dad);
        if (token_tag[0] == "WHILE") return WhileBlock(new_Dad);
        if (token_tag[0] == "FOR") return ForBlock(new_Dad);
        if (token_tag[0] == "CASE") return CaseBlock(new_Dad);
        if (token_tag[0] == "GOTO") {  //GotoBlock;  //GotoBlock → GOTO INTEGER
            add_node(new_Dad, "GotoBlock");
            new_Dad = new_Dad->nodes.back();
            add_node(new_Dad, "goto"); next_token();
            if (token_tag[0] != "NUM_INT") return print_error("ErrorGotoBlock: Supposed to be 'int'.");
            add_node(new_Dad, "v_NUM_INT"); next_token();
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorStmt: Supposed to be ';'.");
            add_node(new_Dad, ";"); next_token(); return true;
        }
        return print_error("ErrorStmt: Supposed to be 'id' or 'read' or 'readln' or 'write' or 'writeln or 'if' or 'else' or 'repeat' or 'while' or 'for' or 'case' or 'goto'.");
    }
    bool Assign(PNode* dad) { //Assign → ID := Expr
        add_node(dad, "Assign");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "ID") return print_error("ErrorAssign: Supposed to be 'id'.");
        add_node(new_Dad,"id"); next_token();
        if (token_tag[0] != "OP_ASIG") return print_error("ErrorAssign: Supposed to be ':='.");
        add_node(new_Dad,":="); next_token();
        return Expr(new_Dad);
    }
    bool Proc(PNode* dad) { //Proc → READ args | READLN args | WRITE args | WRITELN args
        add_node(dad, "Proc");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[1] != "read" && token_tag[1] != "readln" && token_tag[1] != "write" && token_tag[1] != "writeln")
            return print_error("ErrorProc: Supposed to be 'read' or 'readln' or 'write' or 'writeln'.");
        add_node(new_Dad, token_tag[1]); next_token();
        return args(new_Dad);
    }
    bool args(PNode* dad) { //args → (ExprList)
        add_node(dad, "Args");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "DEL_OP") return print_error("ErrorArgs: Supposed to be '('.");
        add_node(new_Dad,"("); next_token();
        if (!ExprList(new_Dad)) return false;
        if (token_tag[0] != "DEL_CP") return print_error("ErrorArgs: Supposed to be ')'.");
        add_node(new_Dad,")"); next_token();
        return true;
    }
    bool IfBlock(PNode* dad) { //IfBlock → IF Expr THEN Stmt ElseBlock
        add_node(dad, "IfBlock");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "IF") return print_error("ErrorIfBlock: Supposed to be 'if'.");
        add_node(new_Dad,"if"); next_token();
        if (!Expr(new_Dad)) return false;
        if (token_tag[0] != "THEN") return print_error("ErrorIfBlock: Supposed to be 'then'.");
        add_node(new_Dad,"then"); next_token();
        if (!Stmt(new_Dad)) return false;
        return ElseBlock(new_Dad);
    }
    bool ElseBlock(PNode* dad) { //ElseBolck → ELSE Stmt | e
        if (token_tag[0] != "ELSE") return true; //Ɛ
        add_node(dad, "ElseBolck");
        PNode* new_Dad = dad->nodes.back();
        add_node(new_Dad,"else"); next_token();
        return Stmt(new_Dad);
    }
    bool RepeatBlock(PNode* dad) { //RepeatBlock → REPEAT StmtList UNTIL Expr
        add_node(dad, "RepeatBlock");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "REPEAT") return print_error("ErrorRepeatBlock: Supposed to be 'repeat'.");
        add_node(new_Dad,"repeat"); next_token();
        if (!StmtList(new_Dad)) return false;
        if (token_tag[0] != "UNTIL") return print_error("ErrorRepeatBlock: Supposed to be 'until'.");
        add_node(new_Dad,"until"); next_token();
        return Expr(new_Dad);
    }
    bool WhileBlock(PNode* dad) { //WhileBlock → WHILE Expr DO BEGIN|e StmtList END|e;
        add_node(dad, "WhileBlock");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "WHILE") return print_error("ErrorWhileBlock: Supposed to be 'while'.");
        add_node(new_Dad,"while"); next_token();
        if (!Expr(new_Dad)) return false;
        if (token_tag[0] != "DO") return print_error("ErrorWhileBlock: Supposed to be 'do'.");
        add_node(new_Dad,"do"); next_token();
        bool is_begin = 0;
        if (token_tag[0] == "BEGIN") add_node(new_Dad,"begin"), next_token(), is_begin = 1; //e
        if (!StmtList(new_Dad)) return false;
        if (token_tag[0] == "END" && is_begin) { //e
            add_node(new_Dad,"end"), next_token();
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorWhileBlock: Supposed to be ';'.");
            add_node(new_Dad,";"); next_token();
        }
        return true;
    }
    bool ForBlock(PNode* dad) { //ForBlock → FOR Assign Direction Expr DO BEGIN|e StmtList END|e; ..... //Direction → TO | DOWNTO
        add_node(dad, "ForBlock");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "FOR") return print_error("ErrorForBlock: Supposed to be 'for'.");
        add_node(new_Dad,"for"); next_token();
        if (!Assign(new_Dad)) return false;
        if (token_tag[0] != "TO" && token_tag[0] != "DOWNTO") return print_error("ErrorForBlock: Supposed to be 'to' or 'downto'.");
        add_node(new_Dad, token_tag[0]); next_token();
        if (!Expr(new_Dad)) return false;
        if (token_tag[0] != "DO") return print_error("ErrorForBlock: Supposed to be 'do'.");
        add_node(new_Dad,"do"); next_token();
        bool is_begin = 0;
        if (token_tag[0] == "BEGIN") add_node(new_Dad,"begin"), next_token(), is_begin = 1; //e
        if (!StmtList(new_Dad)) return false;
        if (token_tag[0] == "END" && is_begin) { //e
            add_node(new_Dad,"end"), next_token();
            if (token_tag[0] != "DEL_SEMC") return print_error("ErrorForBlock: Supposed to be ';'.");
            add_node(new_Dad,";"); next_token();
        }
        return true;
    }
    bool CaseBlock(PNode* dad) { //CaseBlock → CASE Expr OF CaseList END
        add_node(dad, "CaseBlock");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "CASE") return print_error("ErrorCaseBlock: Supposed to be 'case'.");
        add_node(new_Dad,"case"); next_token();
        if (!Expr(new_Dad)) return false;
        if (token_tag[0] != "OF") return print_error("ErrorCaseBlock: Supposed to be 'of'.");
        add_node(new_Dad,"of"); next_token();
        if (!CaseList(new_Dad)) return false;
        if (token_tag[0] != "END") return print_error("ErrorCaseBlock: Supposed to be 'end'.");
        add_node(new_Dad,"end"); next_token();
        return true;
    }
    bool CaseList(PNode* dad) { //CaseList → CaseExpr CaseList | e
        add_node(dad, "CaseList");
        PNode* new_Dad = dad->nodes.back();
        if (!CaseExpr(new_Dad)) return false;
        if (token_tag[0] == "NUM_INT" || token_tag[0] == "NUM_REAL" || token_tag[0] == "STRING" || token_tag[0] == "ID")
            return CaseList(new_Dad); //Ɛ
        return true;
    }
    bool CaseExpr(PNode* dad) { //CaseExpr → Value : Stmt; | ID : Stmt;
        add_node(dad, "CaseExpr");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "NUM_INT" && token_tag[0] != "NUM_REAL" && token_tag[0] != "STRING" && token_tag[0] != "ID")
            return print_error("ErrorCaseExpr: Supposed to be 'int' or 'real' or 'string' or 'id'.");
        add_node(new_Dad,token_tag[0]); next_token();
        if (token_tag[0] != "DEL_COL") return print_error("ErrorCaseExpr: Supposed to be ':'.");
        add_node(new_Dad,":"); next_token();
        if (!Stmt(new_Dad)) return false; //already next token
        if (token_tag[0] != "DEL_SEMC") return print_error("ErrorCaseExpr: Supposed to be ';'.");
        add_node(new_Dad,";"); next_token();
        return true;
    }
    bool ExprList(PNode* dad) { //ExprList → Expr, ExprList | e
        add_node(dad, "ExprList");
        PNode* new_Dad = dad->nodes.back();
        if (!Expr(new_Dad)) return false;
        if (token_tag[0] != "DEL_COM") return true; //e
        add_node(new_Dad,","); next_token();
        return ExprList(new_Dad); //Recursion
    }
    bool Expr(PNode* dad) { //Expr → Expr_ RelOp Expr | Expr_
        add_node(dad, "Expr");
        PNode* new_Dad = dad->nodes.back();
        if (!Expr_(new_Dad)) return false;
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return true;
        if (!RelOp(new_Dad)) return false;
        return Expr(new_Dad); //Recursion
    }
    bool RelOp(PNode* dad) { //RelOp → = | <> | < | <= | >= | >
        add_node(dad, "RelOp");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "OP_EQU" && token_tag[0] != "OP_NOT_EQU" && token_tag[0] != "OP_LT" && token_tag[0] != "OP_LE" && token_tag[0] != "OP_GE" && token_tag[0] != "OP_GT")
            return print_error("ErrorRelOP: Supposed to be '=' or '<>' or '<' or '<=' or '>=' or '>'.");
        add_node(new_Dad, token_tag[0]); next_token();
        return true;
    }
    bool Expr_(PNode* dad) { //Expr_ → Term + Expr_ | Term - Expr_ | Term OR Expr_ | Term
        add_node(dad, "Expr_");
        PNode* new_Dad = dad->nodes.back();
        if (!Term(new_Dad)) return false;
        if (token_tag[0] != "OP_SUM" && token_tag[0] != "OP_RES" && token_tag[0] != "OP_OR")
            return true; //Term
        add_node(new_Dad, token_tag[0]); next_token();
        return Expr_(new_Dad); //Recursion
    }
    bool Term(PNode* dad) { //Term → Factor MUL|DIV|MOD|AND Term    |    Factor
        add_node(dad, "Term");
        PNode* new_Dad = dad->nodes.back();
        if (!Factor(new_Dad)) return false;
        if (token_tag[0] != "OP_MULT" && token_tag[0] != "OP_DIV" && token_tag[0] != "OP_DIV_ENT" && token_tag[0] != "OP_MOD" && token_tag[0] != "OP_AND")
            return true; //Factor
        add_node(new_Dad, token_tag[0]); next_token();
        return Term(new_Dad); //Recursion
    }
    bool Factor(PNode* dad) { //Factor → ID | Value | FUNCTION(ExprList) | (ExprList) | NOT Factor | MINUS Factor
        add_node(dad, "Factor");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] == "FUNCTION") { //FUNCTION(ExprList) 
            add_node(new_Dad,"function"); next_token();
            if (token_tag[0] != "DEL_OP") return print_error("ErrorFactor: Supposed to be '('.");
            add_node(new_Dad,"("); next_token();
            if (!ExprList(new_Dad)) return false;
            if (token_tag[0] != "DEL_CP") return print_error("ErrorFactor: Supposed to be ')'.");
            add_node(new_Dad,")"); next_token();
            return true;
        }
        if (token_tag[0] == "ID" || token_tag[0] == "NUM_INT" || token_tag[0] == "NUM_REAL" || token_tag[0] == "STRING") { //ID | Value
            add_node(new_Dad, "v_"+token_tag[0]); next_token();
            return true;
        }
        if (token_tag[0] == "DEL_OP") { //(ExprList)
            add_node(new_Dad,"("); next_token();
            if (!ExprList(new_Dad)) return false;
            if (token_tag[0] != "DEL_CP") return print_error("ErrorFactor: Supposed to be ')'.");
            add_node(new_Dad,")"); next_token();
            return true;
        }
        if (token_tag[0] == "OP_NOT" || token_tag[0] == "OP_RES") { //NOT Factor  //MINUS Factor
            add_node(new_Dad, token_tag[0]); next_token();
            return Factor(new_Dad);
        }
        return print_error("ErrorFactor: Supposed to be 'id' or 'int' or 'real' or 'string' or 'function' or '(' or 'not' or '-'.");
    }
    bool MainCode(PNode* dad) { //MainCode → BEGIN StmtList END.
        add_node(dad, "MainCode");
        PNode* new_Dad = dad->nodes.back();
        if (token_tag[0] != "BEGIN") return print_error("ErrorMainCode: Supposed to be 'begin'.");
        add_node(new_Dad,"begin"); next_token();
        if (!StmtList(new_Dad)) return false;
        if (token_tag[0] != "END") return print_error("ErrorMainCode: Supposed to be 'end'.");
        add_node(new_Dad,"end"); next_token();
        if (token_tag[0] != "OP_DOT") return print_error("ErrorMainCode: Supposed to be '.'.");
        add_node(new_Dad,"."); next_token();
        return true;
    }
};
void parser() {
    next_token();   
    parser_class parser;
    if (!parser.Program()) return; //Error
    cout << "------------------------------PARSE_TREE------------------------------" << endl;
    parse_tree(tree.root); //Print parse tree
}