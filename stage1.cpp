// Connor Bumann and Ian Park
// CS 4301
// Stage 1

#include <stage1.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <vector>
// functions for <getTime>
// functions for <ostrstream>
// functions for <istrstream>

using namespace std;

Compiler::Compiler(char **argv) // constructor
{
    // open sourceFile using argv[1]
    // open listingFile using argv[2]
    // open objectFile using argv[3]
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
}

Compiler::~Compiler() // destructor
{
   // close all open files
   sourceFile.close();
   listingFile.close();
   objectFile.close();
}

void Compiler::createListingHeader()
{
    time_t now = time (NULL);

    listingFile << "STAGE1: Connor Bumann, Ian Park        "; // add DATE, TIME OF DAY
    listingFile << ctime(&now) << endl;
    
    listingFile << "LINE NO.              SOURCE STATEMENT" << endl;
    lineNo++;
    listingFile << endl << setw(5) << lineNo << "|";
}

// Methods implementing the grammar productions
void Compiler::parser()
{
    //ch must be initialized to the first character of the source file
    //if (nextToken() != "program")
    //    processError("keyword \"program\" expected");
    //a call to nextToken() has two effects
    // (1) the variable, token, is assigned the value of the next token
    // (2) the next token is read from the source file in order to make
    // the assignment. The value returned by nextToken() is also
    // the next token.
    //prog()
    //parser implements the grammar rules, calling first rule
    nextChar();
    nextToken();
    if (token != "program"){
        processError("keyword \"program\" expected");
    }
    prog();
}

void Compiler::createListingTrailer()
{
    // print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED"
    // listingFile << endl;
    // listingFile << left;
    // listingFile << setw(28) << "COMPILATION TERMINATED" << errorCount << " ERRORS ENCOUNTERED";
    string error;
    
    if (errorCount == 1) {
        error = " ERROR";
    }
    else {
        error = " ERRORS";
    }
    listingFile << endl << left << setw(28) << "COMPILATION TERMINATED" << errorCount << error << " ENCOUNTERED" << endl;
}

void Compiler::processError(string err)
{
    // Output err to listingFile
    // Call exit(EXIT_FAILURE) to terminate program
    errorCount++;
    listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
    createListingTrailer();
    exit(EXIT_FAILURE);
}

// Methods implementing the grammar productions
void Compiler::prog() //token should be "program"
{
    if (token != "program")
    {
        processError("keyword \"program\" expected");
    }
    progStmt();
    if (token == "const")
    {
        consts();
    }
    if (token == "var")
    {
        vars();
    }
    if (token != "begin")
    {
      processError("keyword \"begin\" expected");
    }
    beginEndStmt();
    if (token != "$")
    {
      processError("no text may follow \"end\"");
    }
}

void Compiler::progStmt() //token should be "program"
{
    string x;

    if (token != "program")
    {
        processError("keyword \"program\" expected");
    }

    x = nextToken();

    if (!isNonKeyId(token))
    {
        processError("program name expected");
    }

    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }

    nextToken();
    code ("program", x);
    
    insert(x, PROG_NAME,CONSTANT, x, NO, 0);
}
void Compiler::consts() //token should be "const"
{
  if (token != "const")
  {
    processError("keyword \"const\" expected");
  }
  if (!isNonKeyId(nextToken()))
  {
    processError("non-keyword identifier must follow \"const\"");
  }

  constStmts();
}
void Compiler::vars() //token should be "var"
{
    if (token != "var")
    {
        processError("keyword \"var\" expected");
    }
    if (!(isNonKeyId(nextToken())))
    {
        processError("non-keyword identifier must follow \"var\"");
    }
    varStmts();
}
void Compiler::beginEndStmt() //token should be "begin"
{
    if (token != "begin")
    {
        processError("keyword \"begin\" expected");
    }
    
    execStmts();

    if (token != "end")
    {
        processError("keyword \"end\" expected");
    }

    if (nextToken() != ".")
    {
        processError("period expected");
    }
    nextToken();
    code("end", ".");
}
void Compiler::constStmts() //token should be NON_KEY_ID
{
    string x, y;

    if (!isNonKeyId(token))
    {
        processError("non-keyword identifier");
    }

    x = token;

    if (nextToken() != "=")
    {
        processError("\"=\" expected");
    }

    y = nextToken();

    if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && y != "true" && y != "false" && !isInteger(y))
    {
        processError("token to right of \"=\" illegal");
    }
    if (y == "+" || y == "-")
    {
        if (!isInteger(nextToken()))
        {
            processError("integer expected after sign");
        }
        y = y + token;
    }
    if (y == "not")
    {  
        if (!isBoolean(nextToken()) && !isNonKeyId(token))
        {
            processError("boolean or non-keyword identifier expected after \"not\"");
        }
        if (isNonKeyId(token)) {
            
            if(whichType(token) == BOOLEAN){
                if(whichValue(token) == "true"){
                    y = "false";
                }
                else{
                    y = "true";
                }
            }
            else {
                processError("boolean expected after not");
            }
        }
        else
        {
            if (token == "true")
            {
              y = "false";
            }
            else if (token == "false")
            {
               y = "true";
            }
        }
       
    }
    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }
    if (whichType(y) != INTEGER && whichType(y) != BOOLEAN)
    {
        processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
    }
    
    insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);

    x = nextToken();

    if (x != "begin" && x != "var" && !(isNonKeyId(x)))
    {
        processError("non-keyword identifier or \"begin\" or \"var\" expected");
    }
    if (isNonKeyId(x))
    {
        constStmts();
    }
}
void Compiler::varStmts() //token should be NON_KEY_ID
{
    string x, y;

    if(!(isNonKeyId(token)))
    {
        processError("non-keyword identifier expected");
    }

    x = ids();

    if (token != ":")
    {
        processError("\":\" expected");
    }
    
    if ( nextToken() != "integer" && token != "boolean")
    {
        processError("illegal type follows \":\"");
    }

    y = token;

    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }
    
    if (y == "integer") {
        insert(x,INTEGER,VARIABLE,"",YES,1);
    } else {
        insert(x,BOOLEAN,VARIABLE,"",YES,1);
    }
    
    if (!isNonKeyId(nextToken()) && token != "begin")
    {
        processError("non-keyword identifier or \"begin\" expected");
    }

    if (isNonKeyId(token))
    {
        varStmts();
    }
}
string Compiler::ids() //token should be NON_KEY_ID
{
    string temp, tempString;

    if (!(isNonKeyId(token)))
    {
        processError("non-keyword identifier expected");
    }

    tempString = token;
    temp = token;

    if (nextToken() == ",")
    {
        if (!isNonKeyId(nextToken()))
        {
            processError("non-keyword identifier expected");
        }
        tempString = temp + "," + ids();
    }
    return tempString;
}
// TODO STAGE1 START

void Compiler::execStmts() {
    // get the next token and see if its end, and leave function if it is
    if (nextToken() == "end") {
        return;
    }
    if (token != "read" && token != "write" && !isNonKeyId(token)) {
        processError("expected \"read\", \"write\", or non-keyword identifier");
    }
    execStmt();
    execStmts();
}
void Compiler::execStmt() {
    if (isNonKeyId(token)) {
        assignStmt();
    }
    else if (token == "read") {
        readStmt();
    }
    else if (token == "write") {
        writeStmt();
    }
    // I dont think this needs an else but im putting one in anyway!
    // rewrite the process error if needed
    else {
        processError("Error in execStmt");
    }
} 
void Compiler::assignStmt() {
    string x;
    if (!isNonKeyId(token)) {
        processError("expected non-keyword identifier");
    }
    x = token;
    pushOperand(x);
    if (nextToken() != ":=") {
        processError("expected \":=\"");
    }
    pushOperator(token);
    express();
    if (nextToken() != ";") {
        processError("expected \";\"");
    }
    code(popOperator(),popOperand(),popOperand());
}
void Compiler::readStmt() {
    string x;
    if (token != "read") {
        processError("expected \"read\"");
    }
    //Read List Start
    if (nextToken() != "(") {
        processError("expected \"(\"");
    }
    nextToken();
    x = ids();
    if (token != ")") {
        processError("expected \")\"");
    }
    code("read",x);
    //Read List End
    if (nextToken() != ";") {
        processError("expected \";\"");
    }
} 
void Compiler::writeStmt() {
    string x;
    if (token != "write") {
        processError("expected \"write\"");
    }
    // Write List Start
    if (nextToken() != "(") {
        processError("expected \"(\"");
    }
    //if there are issues with this function it may be this
    nextToken();
    x = ids();
    if (token != ")") {
        processError("expected \")\"");
    }
    code("write",x);
    //Write List End
    if (nextToken() != ";") {
        processError("expected \";\"");
    }
}
void Compiler::express() {
    term();
    expresses();
} 
void Compiler::expresses() {
    string x;
    if (nextToken() == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">") {
        x = token;
        pushOperator(x);
        factor();
        string op = popOperator();
        string operand1 = popOperand();
        string operand2 = popOperand();
        code(op, operand1, operand2);
        terms();
    }
} 
void Compiler::term() {
    factor();
    terms();
}
void Compiler::terms() {
    string x;
    if (nextToken() == "+" || token == "-" || token == "or") {
        x = token;
        pushOperator(x);
        factor();
        terms();
    }
}    
void Compiler::factor() {
    if (nextToken() != "not" && token != "+" && token != "-" && !isInteger(token) && !isBoolean(token) && !isNonKeyId(token) && token != "(") {
        processError("expected part");
    }
    part();
    factors();
}    
void Compiler::factors() {
    string x;
    //mult_level_op
    if (nextToken() == "*" || token == "div" || token == "mod" || token == "and") { 
        x = token;
        pushOperator(x);
        nextToken();
        part();
        // check this 
        string op = popOperator();
        string operand1 = popOperand();
        string operand2 = popOperand();
        code(op, operand1, operand2);
        factors();
    }
}   
void Compiler::part() {
    string x = token;
    if (token == "not") {
        if (nextToken() == "(") {
            express();
            if (token != ")") {
                processError("expected \")\"");
            }
            string popped = popOperand();
            code("not",popped);
        }
        else if (isBoolean(x)) {
            if (x == "true") {
                x = "false";
            }
            else {
                x = "true";
            }
            pushOperand(x);
        }
        else if (isNonKeyId(x)) {
            code("not", x);
        }
        else {
            processError("expected \"(\", BOOLEAN, or non-keyword identifier");
        }
    }
    else if (token == "+") {
        if ( nextToken() == "(") {
            express();
            if (token != ")") {
                processError("expected \")\"");
            }
        }
        else if (isInteger(x) || isNonKeyId(x)) {
            pushOperand(x);
        }
        else {
            processError("expected \"(\", INTEGER, or non-keyword identifier");
        }
    }
    else if (token == "-") {
        x = token;
        if ( nextToken() == "(") {
            express();
            if (token != ")") {
                processError("expected \")\"");
            }
            string popped = popOperand();
            code("neg",popped);
        }
        else if (isInteger(x)) {
            pushOperand("-"+x);
        }
        else if (isNonKeyId(x)) {
            code("neg", x);
        }
        else {
            processError("expected \"(\", INTEGER, or non-keyword identifier");
        }
    }
    else if(isInteger(token) || isBoolean(token) || isNonKeyId(token)) {
        pushOperand(token);
    }
    else if(token == "(") {
        express();
        if (token != ")") {
            processError("expected \")\"");
        }
    } 
    else {
        processError("part error");
    }
}   
// TODO STAGE1 END

// Helper functions for the Pascallite lexicon
bool Compiler::isKeyword(string s) const { // determines if s is a keyword
    vector<string> keywords = {"program", "begin", "end", "var", "const", "integer", "boolean", "true", "false", "not", "mod", "div", "or", "read", "write"};
    for(uint i = 0; i < keywords.size(); ++i) {
        if(s == keywords[i])
            return true;
    }
    return false;
}

bool Compiler::isSpecialSymbol(char c) const { // determines if c is a special symbol
    vector<char> symbols = {'=',':',',',';','.','+','-',':','*','(',')','<','>'};
    for (uint i = 0; i < symbols.size(); ++i) {
        if(c == symbols[i]){
            return true;
        }
    }
    return false;
}

bool Compiler::isNonKeyId(string s) const {// determines if s is a non_key_id
    if(isKeyword(s)) {
        return false;
    }
    
    if (!isalpha((char)s[0])) {
        return false;
    }
    for(uint i = 1; i < s.length(); ++i) {
        if((char)s[i] == '_' && (char)s[i+1] != '_' && ( isalpha((char)s[i+1]) || isdigit((char)s[i+1]) )) {
            
        }
        else if (isalpha((char)s[i]) || isdigit((char)s[i])) {
            
        }
        else {
            return false;
        }
    }
    return true;
}

bool Compiler::isInteger(string s) const { // determines if s is an integer
    // num nums!
    if(!isdigit((char)s[0])) {
        return false;
    }
    for(uint i = 1; i < s.length(); ++i) {
        if(!isdigit((char)s[i])) {
            return false;
        }
    }
    return true;
}

bool Compiler::isBoolean(string s) const {  // determines if s is a boolean4
    if (s == "true") {
        return true;
    }
    else if (s == "false") {
        return true;
    }
    else {
        return false;
    }
}

// Issues may happen with the + and - part and maybe even not because of spaces
bool Compiler::isLiteral(string s) const {  // determines if s is a literal
    if(isInteger(s)) {
        return true;
    } 
    else if (s == "true" || s == "false") {
        return true;
    }
    else if (s.find("not") != string::npos) {
        s.erase(s.find("not"), 4);
        if(isBoolean(s)){
            return true;
        }
        return false;
    }
    else if (s.find("+") != string::npos) {
        s.erase(s.find("+"), 1);
        if(isInteger(s)){
            return true;
        }
        return false;
    }
    else if (s.find("-") != string::npos) {
        s.erase(s.find("-"), 1);
        if(isInteger(s)){
            return true;
        }
        return false;
    }
    else {
        return false;
    }
}

// Action routines
void Compiler::insert(string externalName,storeTypes inType, modes inMode, string inValue,allocation inAlloc, int inUnits) 
{
    static int numTableEntries;
    string name;
    name = externalName.substr(0, externalName.find(','));
    if (externalName.find(',') != string::npos){
        externalName.erase(0, externalName.find(',')+1);
    } else {
        externalName = "";
    }
    if (name.length() > 15) {
            name.erase(15,name.length());
    }
    while ( name != "") {
        if (symbolTable.find(name) != symbolTable.end()) {
           processError("symbol "+ name +" is multiply defined");
        }
        else if (isKeyword(name)) {
            processError("illegal use of keyword");
        }
        else { //create table entry
            numTableEntries++;
            if (numTableEntries <= 256) {
                if (isupper(name[0])) {
                    //symbolTable[name]=(name,inType,inMode,inValue,inAlloc,inUnits);
                    SymbolTableEntry ste(name,inType,inMode,inValue,inAlloc,inUnits);
                    symbolTable.insert(pair<string, SymbolTableEntry>(name, ste));
                }
                else {
                    //symbolTable[name]=(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits)
                    SymbolTableEntry ste(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits);
                    symbolTable.insert(pair<string, SymbolTableEntry>(name, ste));
                }
            }
            else {
                processError("Symbol Table overflow");
            }
        }
        name = externalName.substr(0, externalName.find(','));
        if (externalName.find(',') != string::npos){
            externalName.erase(0, externalName.find(',')+1);
        } else {
            externalName = "";
        }
        if (name.length() > 15) {
            name.erase(15,name.length());
        }
    }
}

storeTypes Compiler::whichType(string name) //tells which data type a name has
{
  storeTypes dataType;

  if(isLiteral(name))
  {
      if(isBoolean(name)) 
      {
          dataType = BOOLEAN;
      }
      else
      {
          dataType = INTEGER;
      }
  }
  else if (symbolTable.find(name) != symbolTable.end())
  {
      dataType = symbolTable.at(name).getDataType();
  }
  else
  {
      processError("reference to undefined constant");
  }
  return dataType;
}

string Compiler::whichValue(string name) //tells which value a name has
{
    string value = "";
    if (isLiteral(name))
    {
        value = name;
    }
    else
    {
        if (symbolTable.find(name) != symbolTable.end())
        {
            value = symbolTable.at(name).getValue();
        }
        else
        {
            processError("reference to undefined constant");
        }
    }
    return value;
}
// TODO
void Compiler::code(string op, string operand1, string operand2)
{
    if (op == "program")
    {
        emitPrologue(operand1);
    }
    else if (op == "end")
    {
        emitEpilogue();
    }
    else if (op == "read") {
        emitReadCode(operand1);
    }
    else if (op == "write") {
        emitWriteCode(operand1);
    }
    else if (op == "+") {
        emitAdditionCode(operand1, operand2);
    }
    else if (op == "-") {
        emitSubtractionCode(operand1, operand2);
    }
    else if (op == "neg") {
       emitNegationCode(operand1);
    }
    else if (op == "not") {
       emitNotCode(operand1);
    }
    else if (op == "*") {
       emitMultiplicationCode(operand1, operand2);
    }
    else if (op == "div") {
       emitDivisionCode(operand1, operand2);
    }
    else if (op == "mod") {
       emitModuloCode(operand1, operand2);
    }
    else if (op == "and") {
       emitAndCode(operand1, operand2);
    }
    else if (op == "or") {
       emitOrCode(operand1, operand2);
    }
    else if (op == ":=") {
       emitAssignCode(operand2, operand1);
    }
    else if (op == "=") {
        emitEqualityCode(operand1, operand2);
    }
    else if (op == "<>") {
      emitInequalityCode(operand1, operand2);
    }
    else if (op == "<") {
      emitLessThanCode(operand1, operand2);
    }
    else if (op == "<=") {
      emitLessThanOrEqualToCode(operand1, operand2);
    }
    else if (op == ">") {
      emitGreaterThanCode(operand1, operand2);
    }
    else if (op == ">=") {
      emitGreaterThanOrEqualToCode(operand1, operand2);
    }
    else
    {
        processError("compiler error since function code should not be called with illegal arguments");
    }
}
// TODO STAGE1 START
void Compiler::pushOperator(string op) { //push name onto operatorStk
    operatorStk.push(op);
}

void Compiler::pushOperand(string operand) {
    if (isLiteral(operand) && symbolTable.find(operand) == symbolTable.end()) {
        if (isBoolean(operand)) 
        {
            if (operand == "true")
                symbolTable.insert(pair<string, SymbolTableEntry>(operand, SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "true", YES, 1)));
            else if (operand == "false")
                symbolTable.insert(pair<string, SymbolTableEntry>(operand, SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "false", YES, 1)));
        }
        else
        {
            insert(operand, whichType(operand), CONSTANT, operand, YES, 1);
        }
    }
   operandStk.push(operand);
}
string Compiler::popOperator() { 
   if (!operatorStk.empty()) {
        string popped = operatorStk.top();
        operatorStk.pop();
        return popped;
   }
    else {
        processError("compiler error; operator stack underflow");
        return "";
    }
}
string Compiler::popOperand() {
    if (!operandStk.empty()) {
        string popped = operandStk.top();
        operandStk.pop();
        return popped;
    }
    else {
        processError("compiler error; operand stack underflow");
        return "";
    }
}
// TODO STAGE1 END

// Emit Functions
void Compiler::emit(string label, string instruction, string operands, string comment)
{
  objectFile << left;
  objectFile << setw(8) << label;
  objectFile << setw(8) << instruction;
  objectFile << setw(24) << operands;
  objectFile << comment;
  objectFile << endl;
}
void Compiler::emitPrologue(string progName, string operand2)
{
    time_t now = time (NULL);
// Identifying Comments
    objectFile << left;
    objectFile << setw(30) << "; Connor Bumann, Ian Park" << ctime(&now);
    
// %INCLUDE Directives
    objectFile << "%INCLUDE \"Along32.inc\"" << endl;
    objectFile << "%INCLUDE \"Macros_Along.inc\"" << endl;

// Begining Statements
    if (progName.length() > 15) {
            progName.erase(15,progName.length());
    }
    objectFile << endl;
    emit("SECTION", ".text");
    emit("global", "_start", "", "; program " + progName);
    objectFile << endl;
    emit("_start:");
}
void Compiler::emitEpilogue(string operand1, string operand2)
{
// Write exit statement
    emit("","Exit", "{0}");
    emitStorage();
}

void Compiler::emitStorage()
{
    // writes .data
    objectFile << endl;
    string internalName;
    string instruction;
    string value;
    string comment_name;
    emit("SECTION", ".data");
    //emit(string label, string instruction, string operands, string comment)
    for (map<string, SymbolTableEntry>::iterator steItr = symbolTable.begin(); steItr != symbolTable.end(); ++steItr) {
        if(steItr->second.getAlloc() == YES && steItr->second.getMode() == CONSTANT) {
            comment_name = "; " + steItr->first;
            internalName = steItr->second.getInternalName();
            instruction = "dd";
            if (steItr->second.getDataType() == BOOLEAN) {
                if (steItr->second.getValue() == "true") 
                {
                    value = "-1";
                }
                else
                {
                    value = "0";
                }
            }
            else 
            {
            value = steItr->second.getValue();
            }
            emit(internalName, instruction, value, comment_name);
        }
    }
    objectFile << endl;
    // write .bss
    emit("SECTION", ".bss");
    for (map<string, SymbolTableEntry>::iterator steItr = symbolTable.begin(); steItr != symbolTable.end(); ++steItr) {
        if(steItr->second.getAlloc() == YES && steItr->second.getMode() == VARIABLE) {
            comment_name = "; " + steItr->first;
            internalName = steItr->second.getInternalName();
            instruction = "resd";
            if(steItr->second.getValue() == "") {
                value = "1";
            }
            emit(internalName, instruction, value, comment_name);
        }
    }
}

// TODO STAGE1 START
void Compiler::emitReadCode(string operand, string operand2) {
    // string name
    // while (name is broken from list (operand) and put in name != "")
    // {
        // if name is not in symbol table
            // processError(reference to undefined symbol)
        // if data type of name is not INTEGER
            // processError(can't read variables of this type)
        // if storage mode of name is not VARIABLE
            // processError(attempting to read to a read-only location)
        // emit code to call the Irvine ReadInt function
        // emit code to store the contents of the A register at name
        // set the contentsOfAReg = name
    // }
}
void Compiler::emitWriteCode(string operand, string operand2) {
    // string name
    // static bool definedStorage = false
    // while (name is broken from list (operand) and put in name != "")
    // {
        // if name is not in symbol table
            // processError(reference to undefined symbol)
        // if name is not in the A register
            // emit the code to load name in the A register
            // set the contentsOfAReg = name
        // if data type of name is INTEGER or BOOLEAN
            // emit code to call the Irvine WriteInt function
        // emit code to call the Irvine Crlf function
 // } // end while
}
void Compiler::emitAssignCode(string operand1, string operand2) {         // op2 = op1
    // if types of operands are not the same
        // processError(incompatible types)
    // if storage mode of operand2 is not VARIABLE
        // processError(symbol on left-hand side of assignment must have a storage mode of VARIABLE)
    // if operand1 = operand2 
        // return
    // if operand1 is not in the A register then
        // emit code to load operand1 into the A register
        // emit code to store the contents of that register into the memory location pointed to by operand2
    // set the contentsOfAReg = operand2
    // if operand1 is a temp then free its name for reuse
    //operand2 can never be a temporary since it is to the left of ':='
}
void Compiler::emitAdditionCode(string operand1, string operand2) {       // op2 +  op1

}
void Compiler::emitSubtractionCode(string operand1, string operand2) {    // op2 -  op1

}
void Compiler::emitMultiplicationCode(string operand1, string operand2) { // op2 *  op1

}
void Compiler::emitDivisionCode(string operand1, string operand2) {       // op2 /  op1

}
void Compiler::emitModuloCode(string operand1, string operand2) {         // op2 %  op1

}
void Compiler::emitNegationCode(string operand1, string ) {           // -op1

}
void Compiler::emitNotCode(string operand1, string operand2) {                // !op1

}
void Compiler::emitAndCode(string operand1, string operand2) {            // op2 && op1

}
void Compiler::emitOrCode(string operand1, string operand2) {             // op2 || op1

}
void Compiler::emitEqualityCode(string operand1, string operand2) {       // op2 == op1

}
void Compiler::emitInequalityCode(string operand1, string operand2) {     // op2 != op1

}
void Compiler::emitLessThanCode(string operand1, string operand2) {       // op2 <  op1

}
void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2) { // op2 <= op1

}
void Compiler::emitGreaterThanCode(string operand1, string operand2) {    // op2 >  op1

}
void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) { // op2 >= op1

}
// TODO STAGE1 END

// Lexical routines
char Compiler::nextChar()
{
    static bool insertNewLine = false;
    sourceFile.get(ch);
    if (sourceFile.eof())
    {
        ch = END_OF_FILE;
        return ch;
    }
    if (insertNewLine) {
       lineNo++;
       listingFile << endl << setw(5) << lineNo << "|";
       insertNewLine = false;
    }
 
    if (ch == '\n')
    {
        if (sourceFile.peek() != EOF)
        {
            insertNewLine = true;
        }
        else 
        {
            listingFile << endl;
        }
    }
    else
    {
        listingFile.put(ch);
    }
    
    return ch;
}

string Compiler::nextToken() //returns the next token or end of file marker
{
    token = "";
    while (token == "")
    {
        if (ch == '{') {
            while (nextChar() != END_OF_FILE && ch != '}') 
            {
                
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
            else
                nextChar();
        }
        else if (ch == '}') { 
            processError("'}' cannot begin token");
        }
        else if (isspace(ch)) {
            nextChar(); 
        }
        else if (isSpecialSymbol(ch)) {
            token = ch;
            nextChar();
            // this my cause errors
            if (token == ":" && ch == '=') {
                token += ch;
                nextChar();
            }
            if (token == "<" && (ch == '>' || ch == '=')) {
                token += ch;
                nextChar();
            }
            else if (token == ">" && ch == '=') {
                token += ch;
                nextChar();
            }
                
        }
        else if (islower(ch)) {
            token = ch;
            nextChar();
            while ((isalpha(ch) || isdigit(ch) || ch == '_') && ch != END_OF_FILE)
            {
                token += ch;
                nextChar();
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
        }
        else if (isdigit(ch)) {
            token = ch;
            nextChar();
            while (isdigit(ch) && ch != END_OF_FILE)
            {
                token += ch;
                nextChar();
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
        }
        else if (ch == END_OF_FILE) {
            token = ch;
        }
        else { // Illegal symbol error
            processError("illegal symbol");
        }
    }
    return token;
}


// Other routines
string Compiler::genInternalName(storeTypes stype) const 
{
    static int boolCount = 0;
    static int progCount = 0;
    static int intCount = 0;
    
    string name;
    if (stype == PROG_NAME) {
        name = "P" + to_string(progCount);
        progCount++;
    } 
    else if  (stype == INTEGER) {
        name = "I" + to_string(intCount);
        intCount++;
    }
    else if (stype == BOOLEAN) {
        name = "B" + to_string(boolCount);
        boolCount++;
    }
    
    return name;
}
// TODO STAGE1 START
void Compiler::freeTemp() {
    currentTempNo--;
    if (currentTempNo < -1) {
        processError("compiler error, currentTempNo should be ≥ –1");
    }
}
string Compiler::getTemp()
{
    string temp;
    currentTempNo++;
    temp = "T" + currentTempNo;
    
    if (currentTempNo > maxTempNo) 
    {
        insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
        maxTempNo++;
    }
    return temp;
}
string Compiler::getLabel() {
    static int i = -1;
    i++;
    string label = ".L" + to_string(i);
    return label;
}

bool Compiler::isTemporary(string s) const { // determines if s represents a temporary
    if (s.length() > 1 && s[0] == 'T')
   {
      for (size_t i = 1; i < s.length(); ++i)
      {
         if (!isdigit(s[i]))
         {
            return false;
         }
      }
      return true;
   }
   return false;
}
// TODO STAGE1 END
/*
// Connor Bumann and Ian Park
// CS 4301
// Stage 1

#include <stage1.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <vector>
#include <sstream>
// functions for <getTime>
// functions for <ostrstream>
// functions for <istrstream>

using namespace std;

Compiler::Compiler(char **argv) // constructor
{
    // open sourceFile using argv[1]
    // open listingFile using argv[2]
    // open objectFile using argv[3]
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
}

Compiler::~Compiler() // destructor
{
   // close all open files
   sourceFile.close();
   listingFile.close();
   objectFile.close();
}

void Compiler::createListingHeader()
{
    time_t now = time (NULL);

    listingFile << "STAGE1: Connor Bumann, Ian Park        "; // add DATE, TIME OF DAY
    listingFile << ctime(&now) << endl;
    
    listingFile << "LINE NO.              SOURCE STATEMENT" << endl;
    lineNo++;
    listingFile << endl << setw(5) << lineNo << "|";
}

// Methods implementing the grammar productions
void Compiler::parser()
{
    //ch must be initialized to the first character of the source file
    //if (nextToken() != "program")
    //    processError("keyword \"program\" expected");
    //a call to nextToken() has two effects
    // (1) the variable, token, is assigned the value of the next token
    // (2) the next token is read from the source file in order to make
    // the assignment. The value returned by nextToken() is also
    // the next token.
    //prog()
    //parser implements the grammar rules, calling first rule
    nextChar();
    nextToken();
    if (token != "program"){
        processError("keyword \"program\" expected");
    }
    prog();
}

void Compiler::createListingTrailer()
{
    // print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED"
    // listingFile << endl;
    // listingFile << left;
    // listingFile << setw(28) << "COMPILATION TERMINATED" << errorCount << " ERRORS ENCOUNTERED";
    string error;
    
    if (errorCount == 1) {
        error = " ERROR";
    }
    else {
        error = " ERRORS";
    }
    listingFile << endl << left << setw(28) << "COMPILATION TERMINATED" << errorCount << error << " ENCOUNTERED" << endl;
}

void Compiler::processError(string err)
{
    // Output err to listingFile
    // Call exit(EXIT_FAILURE) to terminate program
    errorCount++;
    listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
    createListingTrailer();
    exit(EXIT_FAILURE);
}

// Methods implementing the grammar productions
void Compiler::prog() //token should be "program"
{
    if (token != "program")
    {
        processError("keyword \"program\" expected");
    }
    progStmt();
    if (token == "const")
    {
        consts();
    }
    if (token == "var")
    {
        vars();
    }
    if (token != "begin")
    {
      processError("keyword \"begin\" expected");
    }
    beginEndStmt();
    if (token != "$")
    {
      processError("no text may follow \"end\"");
    }
}

void Compiler::progStmt() //token should be "program"
{
    string x;

    if (token != "program")
    {
        processError("keyword \"program\" expected");
    }

    x = nextToken();

    if (!isNonKeyId(token))
    {
        processError("program name expected");
    }

    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }

    nextToken();
    code ("program", x);
    
    insert(x, PROG_NAME,CONSTANT, x, NO, 0);
}
void Compiler::consts() //token should be "const"
{
  if (token != "const")
  {
    processError("keyword \"const\" expected");
  }
  if (!isNonKeyId(nextToken()))
  {
    processError("non-keyword identifier must follow \"const\"");
  }

  constStmts();
}
void Compiler::vars() //token should be "var"
{
    if (token != "var")
    {
        processError("keyword \"var\" expected");
    }
    if (!(isNonKeyId(nextToken())))
    {
        processError("non-keyword identifier must follow \"var\"");
    }
    varStmts();
}
void Compiler::beginEndStmt() //token should be "begin"
{
    if (token != "begin")
    {
        processError("keyword \"begin\" expected");
    }
    
    execStmts();

    if (token != "end")
    {
        processError("keyword \"end\" expected");
    }

    if (nextToken() != ".")
    {
        processError("period expected");
    }
    nextToken();
    code("end", ".");
}
void Compiler::constStmts() //token should be NON_KEY_ID
{
    string x, y;

    if (!isNonKeyId(token))
    {
        processError("non-keyword identifier");
    }

    x = token;

    if (nextToken() != "=")
    {
        processError("\"=\" expected");
    }

    y = nextToken();

    if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && y != "true" && y != "false" && !isInteger(y))
    {
        processError("token to right of \"=\" illegal");
    }
    if (y == "+" || y == "-")
    {
        if (!isInteger(nextToken()))
        {
            processError("integer expected after sign");
        }
        y = y + token;
    }
    if (y == "not")
    {  
        if (!isBoolean(nextToken()) && !isNonKeyId(token))
        {
            processError("boolean or non-keyword identifier expected after \"not\"");
        }
        if (isNonKeyId(token)) {
            
            if(whichType(token) == BOOLEAN){
                if(whichValue(token) == "true"){
                    y = "false";
                }
                else{
                    y = "true";
                }
            }
            else {
                processError("boolean expected after not");
            }
        }
        else
        {
            if (token == "true")
            {
              y = "false";
            }
            else if (token == "false")
            {
               y = "true";
            }
        }
       
    }
    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }
    if (whichType(y) != INTEGER && whichType(y) != BOOLEAN)
    {
        processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
    }
    
    insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);

    x = nextToken();

    if (x != "begin" && x != "var" && !(isNonKeyId(x)))
    {
        processError("non-keyword identifier or \"begin\" or \"var\" expected");
    }
    if (isNonKeyId(x))
    {
        constStmts();
    }
}
void Compiler::varStmts() //token should be NON_KEY_ID
{
    string x, y;

    if(!(isNonKeyId(token)))
    {
        processError("non-keyword identifier expected");
    }

    x = ids();

    if (token != ":")
    {
        processError("\":\" expected");
    }
    
    if ( nextToken() != "integer" && token != "boolean")
    {
        processError("illegal type follows \":\"");
    }

    y = token;

    if (nextToken() != ";")
    {
        processError("semicolon expected");
    }
    
    if (y == "integer") {
        insert(x,INTEGER,VARIABLE,"",YES,1);
    } else {
        insert(x,BOOLEAN,VARIABLE,"",YES,1);
    }
    
    if (!isNonKeyId(nextToken()) && token != "begin")
    {
        processError("non-keyword identifier or \"begin\" expected");
    }

    if (isNonKeyId(token))
    {
        varStmts();
    }
}
string Compiler::ids() //token should be NON_KEY_ID
{
    string temp, tempString;

    if (!(isNonKeyId(token)))
    {
        processError("non-keyword identifier expected");
    }

    tempString = token;
    temp = token;

    if (nextToken() == ",")
    {
        if (!isNonKeyId(nextToken()))
        {
            processError("non-keyword identifier expected");
        }
        tempString = temp + "," + ids();
    }
    return tempString;
}
// TODO STAGE1 START
void Compiler::execStmts() {
    // get the next token and see if its end, and leave function if it is
    if (nextToken() == "end") {
        return;
    }
    if (token != "read" && token != "write" && !isNonKeyId(token)) {
        processError("expected \"read\", \"write\", or non-keyword identifier");
    }
    execStmt();
    execStmts();
}
void Compiler::execStmt() {
    if (isNonKeyId(token)) {
        assignStmt();
    }
    else if (token == "read") {
        readStmt();
    }
    else if (token == "write") {
        writeStmt();
    }
    // I dont think this needs an else but im putting one in anyway!
    // rewrite the process error if needed
    else {
        processError("Error in execStmt");
    }
} 
void Compiler::assignStmt() {
    string x;
    if (!isNonKeyId(token)) {
        processError("expected non-keyword identifier");
    }
    x = token;
    pushOperand(x);
    if (nextToken() != ":=") {
        processError("expected \":=\"");
    }
    pushOperator(token);
    express();
    if (nextToken() != ";") {
        processError("expected \":=\"");
    }
    code(popOperator(),popOperand(),popOperand());
}
void Compiler::readStmt() {
    string x;
    if (token == "read") {
        readStmt();
    }
    // Read List Start
    if (nextToken() != "(") {
        processError("expected \"(\"");
    }
    nextToken();
    x = ids();
    
    if (nextToken() != ")") {
        processError("expected \")\"");
    }
    code("read",x);
    // Read List End
    if (nextToken() != ";") {
        processError("expected \";\"");
    }
} 
void Compiler::writeStmt() {
    string x;
    if (token == "write") {
        readStmt();
    }
    // Write List Start
    if (nextToken() != "(") {
        processError("expected \"(\"");
    }
    // if there are issues with this function it may be this
    nextToken();
    x = ids();
    if (nextToken() != ")") {
        processError("expected \")\"");
    }
    code("write",x);
    // Write List End
    if (nextToken() != ";") {
        processError("expected \";\"");
    }
}
void Compiler::express() {
    term();
    expresses();
} 
void Compiler::expresses() {
    string x;
    if (nextToken() == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">") {
        x = token;
        pushOperator(x);
        factor();
        string op = popOperator();
        string operand1 = popOperand();
        string operand2 = popOperand();
        code(op, operand1, operand2);
        terms();
    }
} 
void Compiler::term() {
    factor();
    terms();
}
void Compiler::terms() {
    string x;
    if (nextToken() == "+" || token == "-" || token == "or") {
        x = token;
        pushOperator(x);
        factor();
        terms();
    }
}    
void Compiler::factor() {
    if (nextToken() != "not" && token != "+" && token != "-" && !isInteger(token) && !isBoolean(token) && !isNonKeyId(token) && token != "(") {
        processError("expected part");
    }
    part();
    factors();
}    
void Compiler::factors() {
    string x;
    // mult_level_op
    if (nextToken() == "*" || token == "div" || token == "mod" || token == "and") { 
        x = token;
        pushOperator(x);
        nextToken();
        part();
        // check this 
        string op = popOperator();
        string operand1 = popOperand();
        string operand2 = popOperand();
        code(op, operand1, operand2);
        factors();
    }
}   
void Compiler::part() {
    string x = token;
    if (token == "not") {
        if (nextToken() == "(") {
            express();
            if (nextToken() != ")") {
                processError("expected \")\"");
            }
            string popped = popOperand();
            code("not",popped);
        }
        else if (isBoolean(x)) {
            if (x == "true") {
                x = "false";
            }
            else {
                x = "true";
            }
            pushOperand(x);
        }
        else if (isNonKeyId(x)) {
            code("not", x);
        }
        else {
            processError("expected \"(\", BOOLEAN, or non-keyword identifier");
        }
    }
    else if (token == "+") {
        if ( nextToken() == "(") {
            express();
            if (nextToken() != ")") {
                processError("expected \")\"");
            }
        }
        else if (isInteger(x) || isNonKeyId(x)) {
            pushOperand(x);
        }
        else {
            processError("expected \"(\", INTEGER, or non-keyword identifier");
        }
    }
    else if (token == "-") {
        x = token;
        if ( nextToken() == "(") {
            express();
            if (nextToken() != ")") {
                processError("expected \")\"");
            }
            string popped = popOperand();
            code("neg",popped);
        }
        else if (isInteger(x)) {
            pushOperand("-"+x);
        }
        else if (isNonKeyId(x)) {
            code("neg", x);
        }
        else {
            processError("expected \"(\", INTEGER, or non-keyword identifier");
        }
    }
    else if(isInteger(token) || isBoolean(token) || isNonKeyId(token)) {
        pushOperand(token);
    }
    else if(token == "(") {
        express();
        if (nextToken() != ")") {
            processError("expected \")\"");
        }
    } 
    else {
        processError("part error");
    }
}   
// TODO STAGE1 END

// Helper functions for the Pascallite lexicon
bool Compiler::isKeyword(string s) const { // determines if s is a keyword
    vector<string> keywords = {"program", "begin", "end", "var", "const", "integer", "boolean", "true", "false", "not", "mod", "div", "or", "read", "write"};
    for(uint i = 0; i < keywords.size(); ++i) {
        if(s == keywords[i])
            return true;
    }
    return false;
}

bool Compiler::isSpecialSymbol(char c) const { // determines if c is a special symbol
    vector<char> symbols = {'=',':',',',';','.','+','-',':','*','(',')','<','>'};
    for (uint i = 0; i < symbols.size(); ++i) {
        if(c == symbols[i]){
            return true;
        }
    }
    return false;
}

bool Compiler::isNonKeyId(string s) const {// determines if s is a non_key_id
    if(isKeyword(s)) {
        return false;
    }
    
    if (!isalpha((char)s[0])) {
        return false;
    }
    for(uint i = 1; i < s.length(); ++i) {
        if((char)s[i] == '_' && (char)s[i+1] != '_' && ( isalpha((char)s[i+1]) || isdigit((char)s[i+1]) )) {
            
        }
        else if (isalpha((char)s[i]) || isdigit((char)s[i])) {
            
        }
        else {
            return false;
        }
    }
    return true;
}

bool Compiler::isInteger(string s) const { // determines if s is an integer
    // num nums!
    if(!isdigit((char)s[0])) {
        return false;
    }
    for(uint i = 1; i < s.length(); ++i) {
        if(!isdigit((char)s[i])) {
            return false;
        }
    }
    return true;
}

bool Compiler::isBoolean(string s) const {  // determines if s is a boolean4
    if (s == "true") {
        return true;
    }
    else if (s == "false") {
        return true;
    }
    else {
        return false;
    }
}

// Issues may happen with the + and - part and maybe even not because of spaces
bool Compiler::isLiteral(string s) const {  // determines if s is a literal
    if(isInteger(s)) {
        return true;
    } 
    else if (s == "true" || s == "false") {
        return true;
    }
    else if (s.find("not") != string::npos) {
        s.erase(s.find("not"), 4);
        if(isBoolean(s)){
            return true;
        }
        return false;
    }
    else if (s.find("+") != string::npos) {
        s.erase(s.find("+"), 1);
        if(isInteger(s)){
            return true;
        }
        return false;
    }
    else if (s.find("-") != string::npos) {
        s.erase(s.find("-"), 1);
        if(isInteger(s)){
            return true;
        }
        return false;
    }
    else {
        return false;
    }
}

// Action routines
void Compiler::insert(string externalName,storeTypes inType, modes inMode, string inValue,allocation inAlloc, int inUnits) 
{
    static int numTableEntries;
    string name;
    name = externalName.substr(0, externalName.find(','));
    if (externalName.find(',') != string::npos){
        externalName.erase(0, externalName.find(',')+1);
    } else {
        externalName = "";
    }
    if (name.length() > 15) {
            name.erase(15,name.length());
    }
    while ( name != "") {
        if (symbolTable.find(name) != symbolTable.end()) {
           processError("symbol "+ name +" is multiply defined");
        }
        else if (isKeyword(name)) {
            processError("illegal use of keyword");
        }
        else { //create table entry
            numTableEntries++;
            if (numTableEntries <= 256) {
                if (isupper(name[0])) {
                    //symbolTable[name]=(name,inType,inMode,inValue,inAlloc,inUnits);
                    SymbolTableEntry ste(name,inType,inMode,inValue,inAlloc,inUnits);
                    symbolTable.insert(pair<string, SymbolTableEntry>(name, ste));
                }
                else {
                    //symbolTable[name]=(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits)
                    SymbolTableEntry ste(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits);
                    symbolTable.insert(pair<string, SymbolTableEntry>(name, ste));
                }
            }
            else {
                processError("Symbol Table overflow");
            }
        }
        name = externalName.substr(0, externalName.find(','));
        if (externalName.find(',') != string::npos){
            externalName.erase(0, externalName.find(',')+1);
        } else {
            externalName = "";
        }
        if (name.length() > 15) {
            name.erase(15,name.length());
        }
    }
}

storeTypes Compiler::whichType(string name) //tells which data type a name has
{
  storeTypes dataType;

  if(isLiteral(name))
  {
      if(isBoolean(name)) 
      {
          dataType = BOOLEAN;
      }
      else
      {
          dataType = INTEGER;
      }
  }
  else if (symbolTable.find(name) != symbolTable.end())
  {
      dataType = symbolTable.at(name).getDataType();
  }
  else
  {
      processError("reference to undefined constant");
  }
  return dataType;
}

string Compiler::whichValue(string name) //tells which value a name has
{
    string value = "";
    if (isLiteral(name))
    {
        value = name;
    }
    else
    {
        if (symbolTable.find(name) != symbolTable.end())
        {
            value = symbolTable.at(name).getValue();
        }
        else
        {
            processError("reference to undefined constant");
        }
    }
    return value;
}
// TODO
void Compiler::code(string op, string operand1, string operand2)
{
    if (op == "program")
    {
        emitPrologue(operand1);
    }
    else if (op == "end")
    {
        emitEpilogue();
    }
    // else if (op == "read")
    // emit read code
    // else if (op == "write")
    // emit write code
    // else if (op == "+") // this must be binary '+'
    // emit addition code
    // else if (op == "-") // this must be binary '-'
    // emit subtraction code
    // else if (op == "neg") // this must be unary '-'
    // emit negation code;
    // else if (op == "not")
    // emit not code
    // else if (op == "*")
    // emit multiplication code
    // else if (op == "div")
    // emit division code
    // else if (op == "mod")
    // emit modulo code
    // else if (op == "and")
    // emit and code
    // ...
    // else if (op == "=")
    // emit equality code
    // else if (op == ":=")
    // emit assignment code
    else
    {
        processError("compiler error since function code should not be called with illegal arguments");
    }
}
// TODO STAGE1 START
void Compiler::pushOperator(string op) { //push name onto operatorStk
    // push name onto stack;
}
string Compiler::popOperator() { //push name onto operandStk
//if name is a literal, also create a symbol table entry for it
    // if name is a literal and has no symbol table entry
        // insert symbol table entry, call whichType to determine the data type of the literal
    // push name onto stack;
    return "string";
}
void Compiler::pushOperand(string operand) {
    // if operatorStk is not empty
        // return top element removed from stack;
    // else
        // processError(compiler error; operator stack underflow)
}
string Compiler::popOperand() {
    // if operandStk is not empty
        // return top element removed from stack;
    // else
        // processError(compiler error; operand stack underflow)
    return "string";
}
// TODO STAGE1 END

// Emit Functions
void Compiler::emit(string label, string instruction, string operands, string comment)
{
  objectFile << left;
  objectFile << setw(8) << label;
  objectFile << setw(8) << instruction;
  objectFile << setw(24) << operands;
  objectFile << comment;
  objectFile << endl;
}
void Compiler::emitPrologue(string progName, string operand2)
{
    time_t now = time (NULL);
// Identifying Comments
    objectFile << left;
    objectFile << setw(30) << "; Connor Bumann, Ian Park" << ctime(&now);
    
// %INCLUDE Directives
    objectFile << "%INCLUDE \"Along32.inc\"" << endl;
    objectFile << "%INCLUDE \"Macros_Along.inc\"" << endl;

// Begining Statements
    if (progName.length() > 15) {
            progName.erase(15,progName.length());
    }
    objectFile << endl;
    emit("SECTION", ".text");
    emit("global", "_start", "", "; program " + progName);
    objectFile << endl;
    emit("_start:");
}
void Compiler::emitEpilogue(string operand1, string operand2)
{
// Write exit statement
    emit("","Exit", "{0}");
    emitStorage();
}

void Compiler::emitStorage()
{
    // writes .data
    objectFile << endl;
    string internalName;
    string instruction;
    string value;
    string comment_name;
    emit("SECTION", ".data");
    //emit(string label, string instruction, string operands, string comment)
    for (map<string, SymbolTableEntry>::iterator steItr = symbolTable.begin(); steItr != symbolTable.end(); ++steItr) {
        if(steItr->second.getAlloc() == YES && steItr->second.getMode() == CONSTANT) {
            comment_name = "; " + steItr->first;
            internalName = steItr->second.getInternalName();
            instruction = "dd";
            if (steItr->second.getDataType() == BOOLEAN) {
                if (steItr->second.getValue() == "true") 
                {
                    value = "-1";
                }
                else
                {
                    value = "0";
                }
            }
            else 
            {
            value = steItr->second.getValue();
            }
            emit(internalName, instruction, value, comment_name);
        }
    }
    objectFile << endl;
    // write .bss
    emit("SECTION", ".bss");
    for (map<string, SymbolTableEntry>::iterator steItr = symbolTable.begin(); steItr != symbolTable.end(); ++steItr) {
        if(steItr->second.getAlloc() == YES && steItr->second.getMode() == VARIABLE) {
            comment_name = "; " + steItr->first;
            internalName = steItr->second.getInternalName();
            instruction = "resd";
            if(steItr->second.getValue() == "") {
                value = "1";
            }
            emit(internalName, instruction, value, comment_name);
        }
    }
}

// TODO STAGE1 START
void Compiler::emitReadCode(string operand, string = "") 
{
   string name = "";
   istringstream tokenStream(operand);
   
   while (getline(tokenStream, name, ","))
   {
      name = trim(name);
      
      if (symbolTable.find(name) == symbolTable.end())
      {
         processError("reference to undefined symbol: " + operand);
      }
      
      if (symbolTable.at(name).getDataType() != INTEGER)
      {
         processError("can't read variables of this type" + name);
      }
      if (symbolTable.at(name).getMode() != VARIABLE)
      {
         processError("attempting to read to a read-only location");
      }
      
      emit("", "CALL", "ReadInt");
      
      emit("", "MOV", name + ", EAX");
      
      contentsOfAReg = name;
   }
}
void Compiler::emitWriteCode(string operand, string = "") {
   string name = "";
   istringstream tokenStream(operand);
   
   while (getline(tokenStream, name, ','))
   {
      name = trim(name);

      if (symbolTable.find(name) == symbolTable.end())
      {
         processError("reference to undefinded symbol");
      }
      if (contentsOfAReg != name)
      {
         emit("Load " + name);
         contentsOfAReg = name;
      }
if (symbolTable.at(name).getDataType() == INTEGER || 
    symbolTable.at(name).getDataType() == BOOLEAN) 
      {
         emit("CALL WriteInt");
      }
      
      emit ("CALL Crlf");
      
   }
}
void Compiler::emitAssignCode(string operand1, string operand2) 
{         
   if (symbolTable[operand1].dataType != symbolTable[operand2].dataType)
   {
      processError("incompatible types");
   }  
   if (symbolTable[operand2].storageMode != "VARIABLE")
   {
      processError("(symbol on left-hand side of assignment must have a storage mode of VARIABLE");      
   }
   if (operand1 == operand2)
   {
      return;
   }
   if (contentsOfAReg != operand1)
   {
      emit("LOAD " + operand1);
      contentsOfAReg = operand1;
   }
   
   emit("STORE " + operand2);
   contentsOfAReg = operand2;
   
   if (isTemporary(operand1))
   {
      freeTemp();
   }
}
void Compiler::emitAdditionCode(string operand1, string operand2) {       
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
   {
      processError("illegal type");
   }
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
   {
      emit("STORE " + contentsOfAReg);
      symbolTable[contentsOfAReg].isAllocated = true;
      deassign(contentsOfAReg);
   }
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
   {
      deassign(contentsOfAReg);
   }
   if (contentsOfAReg != operand1 && contentsOfAReg != operand2)
   {
      emit("Load " + operand2);
      contentsOfAReg = operand2;
   }
   
   emit("ADD " + operand1);
   
   
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   string temp = getTemp();
   emit("STORE " + temp);
   symbolTable[temp].setDataType(INTEGER);
   contentsOfAReg = temp;
   
   operandStk.push(temp);
   
}
void Compiler::emitSubtractionCode(string operand1, string operand2) 
{
   // type check
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg );
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit ("", "SUB", operand1);
   
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   string temp = getTemp();
   emit("STORE " + temp);
   symbolTable[temp].setDataType(INTEGER);
   contentsOfAReg = temp;
   
   operandStk.push(temp);
}
void Compiler::emitMultiplicationCode(string operand1, string operand2) 
{ 
   // type check
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg );
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   // multiply
   emit("", "MULT", operand1);
   
      // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // save result
   string temp = getTemp();
   symbolTable[temp] = SymbolTableEntry(temp, INTEGER, VARIABLE, "", YES, 1);
   contentsOfAReg = temp;
   
   // push result
   pushOperand(temp);
}
void Compiler::emitDivisionCode(string operand1, string operand2) 
{    
   // type check
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   // divide
   emit("", "CDQ", "" );
   emit("", "IDIV", operand1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // save result
   string temp = getTemp();
   symbolTable[temp] = SymbolTableEntry(temp, INTEGER, VARIABLE, "", YES, 1);
   contentsOfAReg = temp;
   
   // push result
   pushOperand(temp);
}
void Compiler::emitModuloCode(string operand1, string operand2) 
{         
   // type check
   if (whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg );
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "CDQ");
   emit("", "IDIV", operand1);
   
   string temp = getTemp();
   emit("", "STORE", "EDX");
   symbolTable[temp] = SymbolTableEntry(temp, INTEGER, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitNegationCode(string operand1, string = "")
{ 
   // type check
   if (whichType(operand1) != INTEGER)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand1)
   {
      emit("LOAD", operand1);
      contentsOfAReg = operand1;
   }
   
   emit("", "NEG");
   
   string temp = getTemp();
   emit("", "STORE", temp);
   symbolTable[temp] = SymbolTableEntry(temp, INTEGER, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitNotCode(string operand1, string = "") 
{                
   // type check
   if (whichType(operand1) != BOOLEAN)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand1)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand1)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand1)
   {
      emit("LOAD", operand1);
      contentsOfAReg = operand1;
   }
   
   emit("" "NOT");
   
   string temp = getTemp();
   emit("", "STORE", temp);
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE,"", YES, 1);
   
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitAndCode(string operand1, string operand2) 
{            
   // type check
   if (whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "AND", operand1);
   
   string temp = getTemp();
   emit("", "STORE", temp);
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitOrCode(string operand1, string operand2) 
{             
   // type check
   if (whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "OR", operand1);
   
   string temp = getTemp();
   emit("", "STORE", temp);
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitEqualityCode(string operand1, string operand2)
{       
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETE", temp);
   
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitOrCode(string operand1, string operand2) 
{             
   // type check
   if (whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN)
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "OR", operand1);
   
   string temp = getTemp();
   emit("", "STORE", temp);
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE,"", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
   
}
void Compiler::emitInequalityCode(string operand1, string operand2) 
{     
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETNE", temp);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
      
}
void Compiler::emitLessThanCode(string operand1, string operand2)
{      
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETL", temp);
   
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE, "", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2)
{ 
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETLE", temp);
   
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE, "", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitGreaterThanCode(string operand1, string operand2) 
{    
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETG", temp);
   
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE, "", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) 
{ 
   // type check
   if (whichType(operand1) != whichType(operand2))
   {
      processError("illegal type");
   }
   // A register
   if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      emit("Store", contentsOfAReg);
      symbolTable[contentsOfAReg].setAlloc(YES);
      contentsOfAReg = "";
   }
   // desassign
   if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
   {
      contentsOfAReg = "";
   }
   
   // load operand 2 into register
   if (contentsOfAReg != operand2)
   {
      emit("LOAD", operand2);
      contentsOfAReg = operand2;
   }
   emit("", "CMP", operand1);
   string temp = getTemp();
   emit("", "SETGE", temp);
   
   symbolTable[temp] = SymbolTableEntry(temp, BOOLEAN, VARIABLE, "", YES, 1);
   
   // clear temp
   if (isTemporary(operand1)) 
   {
       freeTemp();  
   }
   if (isTemporary(operand2)) 
   {
       freeTemp();  
   }
   
   // update A register
   contentsOfAReg = temp;
   pushOperand(temp);
}
// TODO STAGE1 END

// Lexical routines
char Compiler::nextChar()
{
    static bool insertNewLine = false;
    sourceFile.get(ch);
    if (sourceFile.eof())
    {
        ch = END_OF_FILE;
        return ch;
    }
    if (insertNewLine) {
       lineNo++;
       listingFile << endl << setw(5) << lineNo << "|";
       insertNewLine = false;
    }
 
    if (ch == '\n')
    {
        if (sourceFile.peek() != EOF)
        {
            insertNewLine = true;
        }
        else 
        {
            listingFile << endl;
        }
    }
    else
    {
        listingFile.put(ch);
    }
    
    return ch;
}

string Compiler::nextToken() //returns the next token or end of file marker
{
    token = "";
    while (token == "")
    {
        if (ch == '{') {
            while (nextChar() != END_OF_FILE && ch != '}') 
            {
                
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
            else
                nextChar();
        }
        else if (ch == '}') { 
            processError("'}' cannot begin token");
        }
        else if (isspace(ch)) {
            nextChar(); 
        }
        else if (isSpecialSymbol(ch)) {
            token = ch;
            nextChar();
        }
        else if (islower(ch)) {
            token = ch;
            nextChar();
            while ((isalpha(ch) || isdigit(ch) || ch == '_') && ch != END_OF_FILE)
            {
                token += ch;
                nextChar();
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
        }
        else if (isdigit(ch)) {
            token = ch;
            nextChar();
            while (isdigit(ch) && ch != END_OF_FILE)
            {
                token += ch;
                nextChar();
            }
            if (ch == END_OF_FILE)
                processError("unexpected end of file");
        }
        else if (ch == END_OF_FILE) {
            token = ch;
        }
        else { // Illegal symbol error
            processError("illegal symbol");
        }
    }
    return token;
}


// Other routines
string Compiler::genInternalName(storeTypes stype) const 
{
    static int boolCount = 0;
    static int progCount = 0;
    static int intCount = 0;
    
    string name;
    if (stype == PROG_NAME) {
        name = "P" + to_string(progCount);
        progCount++;
    } 
    else if  (stype == INTEGER) {
        name = "I" + to_string(intCount);
        intCount++;
    }
    else if (stype == BOOLEAN) {
        name = "B" + to_string(boolCount);
        boolCount++;
    }
    
    return name;
}
// TODO STAGE1 START
void Compiler::freeTemp() {
    currentTempNo--;
    if (currentTempNo < -1) {
        processError("compiler error, currentTempNo should be ≥ –1");
    }
}
string Compiler::getTemp()
{
    string temp;
    currentTempNo++;
    temp = "T" + currentTempNo;
    
    if (currentTempNo > maxTempNo) 
    {
        insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
        maxTempNo++;
    }
    return temp;
}
string Compiler::getLabel() {
    static int i = -1;
    i++;
    string label = ".L" + to_string(i);
    return label;
}

bool Compiler::isTemporary(string s) const { // determines if s represents a temporary
    if (s.length() > 1 && s[0] == 'T')
   {
      for (size_t i = 1; i < s.length(); ++i)
      {
         if (!isdigit(s[i]))
         {
            return false;
         }
      }
      return true;
   }
   return false;
}
*/
