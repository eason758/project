#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAXLEN 1002

typedef enum {MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, DIVIDEZERO, MULTIPLEASSIGN, WRONGASSIGN,WRONGEND, NUMINTTHEFRONT} ErrorType;
typedef enum {UNKNOWN, END, INT, ID, ORANDXOR, ADDSUB, MULDIV, ASSIGN,
LPAREN, RPAREN, ENDFILE} TokenSet;

typedef struct _Node {
    char lexeme[MAXLEN];
    TokenSet data;
    int val;
    struct _Node *left, *right;
    bool can_use;
} BTNode ;

TokenSet lookahead = UNKNOWN;
char lexeme[MAXLEN];

BTNode* factor(void);
BTNode* term(void);
BTNode* term_tail(BTNode *);
BTNode* expr(void);
BTNode* expr_tail(BTNode *);

void statement(int);
char* getLexeme(void) ;
TokenSet getToken(void) ;
void advance(void) ;
void error(ErrorType) ;
int match(TokenSet) ;
void freeTree(BTNode *);
BTNode* makeNode(TokenSet, const char *);

//沒使用到的*****************************
void printPrefix(BTNode *) ;
int evaluateTree(BTNode *,int) ;
int getval();
void setval(char *, int);
//***************************************


//自己新增的***************************************
void printCODE(BTNode *, int); //print assembly code
int get_memory(char *);  //得出一個記憶體空間(4n)

typedef struct _var{
    char var_list[MAXLEN];      //儲存出現過的 variable
    int val;
    bool can_use;
}var;

int count = 3;
var table[65];
bool assign;                    //判斷這行statement是否已經assign過了
void get_out();
int total_statement = 0;
BTNode* glo_retp[MAXLEN];

var registor_storage[8];     //暫時儲存在registor的變數們
bool reg_store_flag[8];      //確認這格registor是否有儲存變數
void reg_to_mem(int);
int print_new_var = 0;       //print的時候增加的新變數(無論在reg或mem)
int assembly_count = 3;         //count mem數量
bool x_y_z_stored[3];       //x y z 有沒有被存進過mem
var table2[65];

bool x_y_z_can_use[3];
//***************************************


int main(void)
{

    //freopen( "testcase.txt" , "r" , stdin ) ;
    //freopen( "output.txt" , "w" , stdout ) ;

    strcpy(table[0].var_list, "x");
    table[0].val = 9873498;
    x_y_z_stored[0] = false;
    table[0].can_use = false;

    strcpy(table[1].var_list, "y");
    table[1].val = 136134;
    x_y_z_stored[1] = false;
    table[1].can_use = false;

    strcpy(table[2].var_list, "z");
    table[2].val = 3213848;
    x_y_z_stored[2] = false;
    table[2].can_use = false;

    for(int i = 3 ; i < 65 ; i++){
        table[i].can_use = true;
    }

    while (1) {
        statement(total_statement);
    }
    return 0;
}

int getval(void){
    int retval;
    int i;
    if (match(INT)) {
        retval = atoi(getLexeme());
    }
    else if (match(ID)) {

        for(i = 0; strcmp(getLexeme(), table[i].var_list) != 0 && i < count ; i++); //找vat_list有沒有出現過一樣的變數

        if(i == count) error(NOTFOUND);//第一次出現在右邊的變數 error
        else retval = table[i].val;

    }
    return retval;
}

void setval(char *str, int val){
    int i;
    for(i = 0; strcmp(str, table[i].var_list) != 0 && i < count ; i++); //找vat_list有沒有出現過一樣的變數

    if(i==count){
        if(count>64) error(RUNOUT);
        else strcpy(table[i].var_list, str);
        count++;

    }
    table[i].val = val;
    return;
}

int evaluateTree(BTNode *root,int dep)
{
    int i;
    if(dep>7) error(RUNOUT);
    int retval = 0, lv, rv;
    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:
        case INT:
            retval = root->val;
            break;
        case ASSIGN:
            lv = evaluateTree(root->left,0);
            retval = 0;
            setval(root->right->lexeme, lv);
            if(root->left->can_use){
                for(i = 0 ; strcmp(table[i].var_list, root->right->lexeme) != 0;i++);
                table[i].can_use = true;
            }
            else{
                for(i = 0 ; strcmp(table[i].var_list, root->right->lexeme) != 0;i++);
                table[i].can_use = false;
            }

            break;
        case ADDSUB:
        case MULDIV:
        case ORANDXOR:
            lv = evaluateTree(root->left,0);
            rv = evaluateTree(root->right,dep+1);
            if (strcmp(root->lexeme, "+") == 0)
                retval = lv + rv;
            else if (strcmp(root->lexeme, "-") == 0)
                retval = lv - rv;
            else if (strcmp(root->lexeme, "*") == 0)
                retval = lv * rv;
            else if (strcmp(root->lexeme, "/") == 0){
                if(rv == 0) error(DIVIDEZERO);                      //可能1/(1/100) = 1/0 (int特性)
                retval = lv / rv;
            }

            else if((strcmp(root->lexeme, "|") == 0))
                retval = lv | rv;
            else if((strcmp(root->lexeme, "&") == 0))
                retval = lv & rv;
            else if((strcmp(root->lexeme, "^") == 0))
                retval = lv ^ rv;
            if(root->left->can_use && root->right->can_use) root->can_use = true;
            else root->can_use = false;
            break;
        default:
            retval = 0;
        }
    }
    root->val = retval;
    return retval;
}

/* create a node without any child.*/
BTNode* makeNode(TokenSet tok, const char *lexe){
    BTNode *node = (BTNode*) malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    int i;

    if(node->data == ID){
        for(i = 0;strcmp(table[i].var_list, lexe) != 0 && i<count; i++);
        if(i==count) node->can_use = true;
        else if(table[i].can_use) node->can_use = true;
        else node->can_use = false;
    }
    else node->can_use = true;

    return node;
}

TokenSet getToken(void)
{
    int i;
    char c;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );  // 忽略空白字元

    if (isdigit(c)) {//判斷收到一個數字(所以最後會回傳INT) 中間這些只是在推字符偵測而已
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i<MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);//清空緩衝區
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+' || c == '-') {//判斷收到一個ADD 或 SUB
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    }
    else if (c == '|' || c == '&' || c =='^') {//判斷收到一個OR AND XOR
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ORANDXOR;
    }
    else if (c == '*' || c == '/') {//判斷收到MUL DIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n') {//判斷一行算式結束
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=') {    //判斷收到 "="(ASSIGN)
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(') {    //判斷收到 "(" (LPAREN)
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')') {    //判斷收到")" (RPAREN)
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_'){ //判斷收到變數 所以回傳ID (中間這些只是在推字符偵測而已)
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_') {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';//清空緩衝區
        return ID;
    }
    else if (c == EOF) {//判斷收到整個測資結束了
        return ENDFILE;
    }
    else {
        return UNKNOWN;//判斷收到奇怪的東西
    }
}

/* factor := INT | ADD_SUB INT | ADD_SUB ID | ID ASSIGN expr| ID | LPAREN expr RPAREN */

BTNode* factor(void)
{
    BTNode* retp = NULL;
    char tmpstr[MAXLEN];

    if(!assign){
        if (match(ID)){
            BTNode*right = makeNode(ID, getLexeme());
            strcpy(tmpstr, getLexeme());
            advance();
            if (match(ASSIGN)) {
                assign = true;
                retp = makeNode(ASSIGN, getLexeme());
                advance();
                retp->right = right;
                retp->left = expr();
            }
            else {
                error(WRONGASSIGN);
            }
        }
        else error(NUMINTTHEFRONT);
    }
    else{
        if (match(INT)) {
            retp =  makeNode(INT, getLexeme());
            retp->val = getval();
            advance();
        }
        else if (match(ID)) {
            retp = makeNode(ID, getLexeme());
            retp->val = getval();
            strcpy(tmpstr, getLexeme());
            advance();
        }
        else if (match(ADDSUB)) {
            strcpy(tmpstr, getLexeme());
            advance();
            if (match(ID) || match(INT)) {
                retp = makeNode(ADDSUB, tmpstr);
                if (match(ID))
                    retp->right = makeNode(ID, getLexeme());
                else
                    retp->right = makeNode(INT, getLexeme());

                retp->right->val = getval();
                retp->left = makeNode(INT, "0");
                retp->left->val = 0;
                advance();
            }
            else {
                error(NOTNUMID);
            }
        }
        else if (match(ORANDXOR)){// ??????不合格????應該error???
            error(NOTNUMID);
        }
        else if (match(LPAREN)) {
            advance();
            retp = expr();
            if (match(RPAREN)) {
                advance();
            }
            else {
                error(MISPAREN);//結束了expr接著再偵測卻沒發現括弧>出問題
            }
        }
        else if (match(ASSIGN)) {
            error(MULTIPLEASSIGN);
        }
        else {
            error(NOTNUMID);
        }
    }
    return retp;
}

/* term := factor term_tail */
BTNode* term(void)
{
    BTNode *node;

    node = factor();

    return term_tail(node);
}

/* term_tail := MUL_DIV factor term_tail|NiL */
BTNode* term_tail(BTNode *left)
{
    BTNode *node;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());

            advance();
            node->left = left;
            node->right = factor();

        return term_tail(node);
    }
    else if(match(ASSIGN)) error(MULTIPLEASSIGN);
    else return left;
}

/* expr := term expr_tail */
BTNode* expr(void)
{
    BTNode *node;

    node = term();

    return expr_tail(node);
}

/* expr_tail := ADD_SUB_AND_OR_XOR term expr_tail | NiL */
BTNode* expr_tail(BTNode *left)
{
    BTNode *node;

    if (match(ORANDXOR)) {
        node = makeNode(ORANDXOR, getLexeme());
        advance();

        node->left = left;
        node->right = term();

        return expr_tail(node);
    }
    else if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();

        node->left = left;
        node->right = term();

        return expr_tail(node);
    }
    else if(match(ASSIGN)) error(MULTIPLEASSIGN);
    else return left;
}

void advance(void)//啟動getToken()來抓取前端的那一項目是什麼token 並賦予給lookahead
{
    lookahead = getToken();
}

int match(TokenSet token) //判斷這個 token 跟 lookahead是否一樣
{
    if (lookahead == UNKNOWN) advance();
    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}

/* statement := END | expr END */

void statement(int i)//只處理一行數學算式
{
    assign = false;             //初始化 代表第一次assign還沒出現
    if (match(ENDFILE)) {       //最終結尾(如果平安完成測資且沒有exit 1的話)
        if(total_statement > 0) get_out();
        else{
            printf("MOV r0 [0]\n");
            printf("MOV r1 [4]\n");
            printf("MOV r2 [8]\n");
            printf("EXIT 0\n");
            exit(0);
        }
    }
    else if (match(END)) {

        advance();          //get 到 換行 就再接收一次 然後再等while回來

    }
    else {

        glo_retp[i] = expr();
        if (match(END)) {
            evaluateTree(glo_retp[i],0);
            //printPrefix(glo_retp[i]); printf("\n");
            total_statement++;
            advance();
        }
        else {
            error(WRONGEND);
        }
    }
}


void printPrefix(BTNode *root)
{
    if (root != NULL)
    {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}


void printCODE(BTNode *node, int reg){
    int i;
    int left_reg, right_reg;
    char op[10];

    switch(node->data){
        case ASSIGN:
            if(node->left->can_use){
                for(i = 0; strcmp(node->right->lexeme, registor_storage[i].var_list) != 0 && i<8 ; i++);
                if(i<8){
                    printf("MOV r%d %d\n",i,node->left->val);
                }
                else{//不在reg裡面
                    printf("MOV r%d %d\n",reg, node->left->val);

                    if(strcmp(node->right->lexeme, "x") == 0){
                        if(x_y_z_stored[0]) printf("MOV [0] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else if(strcmp(node->right->lexeme, "y") == 0){
                        if(x_y_z_stored[4]) printf("MOV [4] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else if(strcmp(node->right->lexeme, "z") == 0){
                        if(x_y_z_stored[8]) printf("MOV [8] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else{
                        for(i = 3; strcmp(node->right->lexeme, table2[i].var_list) != 0 && i<assembly_count ; i++);

                        if(i<assembly_count)printf("MOV [%d] r%d\n",4*i,reg);
                        else {
                            print_new_var++;
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                        }
                    }
                }
            }
            else{
                printCODE(node->left, reg);
                for(i = 0; strcmp(node->right->lexeme, registor_storage[i].var_list) != 0 && i<8 ; i++);
                if(i<8){
                    if(i != reg)printf("MOV r%d r%d\n",i,reg);
                }
                else{//不在reg裡面

                    if(strcmp(node->right->lexeme, "x") == 0){
                        if(x_y_z_stored[0]) printf("MOV [0] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else if(strcmp(node->right->lexeme, "y") == 0){
                        if(x_y_z_stored[4]) printf("MOV [4] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else if(strcmp(node->right->lexeme, "z") == 0){
                        if(x_y_z_stored[8]) printf("MOV [8] r%d\n",reg);
                        else{
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                            print_new_var++;
                        }
                    }
                    else{
                        for(i = 3; strcmp(node->right->lexeme, table2[i].var_list) != 0 && i<assembly_count ; i++);

                        if(i<assembly_count)printf("MOV [%d] r%d\n",4*i,reg);
                        else {
                            print_new_var++;
                            reg_store_flag[reg] = true;
                            strcpy(registor_storage[reg].var_list , node->right->lexeme);
                        }
                    }
                }
            }
            break;

        case ORANDXOR:
        case ADDSUB:
        case MULDIV:
            left_reg = reg;
            right_reg = (reg+1)%8;
            switch(node->lexeme[0]){
                case '^':
                    strcpy(op, "XOR");
                    break;
                case '|':
                    strcpy(op, "OR");
                    break;
                case '&':
                    strcpy(op, "AND");
                    break;
                case '+':
                    strcpy(op, "ADD");
                    break;
                case '-':
                    strcpy(op, "SUB");
                    break;
                case '*':
                    strcpy(op, "MUL");
                    break;
                case '/':
                    strcpy(op, "DIV");
                    break;
            }
            if(node->can_use){
                printf("MOV r%d %d\n",left_reg,node->val);                                        //0.1的問題?
            }
            else{
                printCODE(node->left,left_reg);
                printCODE(node->right,right_reg);
                printf("%s r%d r%d\n", op, left_reg,right_reg);
            }
            break;

        case ID:

            if(reg_store_flag[reg]){
                if(strcmp(registor_storage[reg].var_list, node->lexeme) == 0) return;
                else{
                    reg_to_mem(reg);    // now registor[reg] is clear
                }
            }
            for(i = 0; strcmp(node->lexeme, registor_storage[i].var_list) != 0 && i<8 ; i++);

            if(i==8)printf("MOV r%d [%d]\n", reg, get_memory(node->lexeme)); //id在mem 從mem借給指定reg
            else    printf("MOV r%d r%d\n", reg, i);                         //id在reg array 搬給指定reg

            break;

        case INT:

            if(reg_store_flag[reg]) reg_to_mem(reg);    // problem 2

            printf("MOV r%d %s\n", reg, node->lexeme);
    }
    return;
}

int get_memory(char * lex){

    if(strcmp(lex, "x") == 0) return 0;
    if(strcmp(lex, "y") == 0) return 4;
    if(strcmp(lex, "z") == 0) return 8;

    int i;

    for(i = 3; strcmp(lex, table2[i].var_list) != 0 && i<=count ; i++); //table的順序(記憶體數字)其實跟print方面的一樣(有待查證)
    if(i<count)return 4*i;

return 4*i;
}

void error(ErrorType errorNum)
{
    printf("EXIT 1\n");/*
    switch (errorNum) {

        case MISPAREN:
            printf("Mismatched parenthesis\n");
            break;

        case NOTNUMID:
            printf("Number or identifier expected\n");
            break;

        case NOTFOUND:
            printf("%s not defined\n", getLexeme());
            break;

        case RUNOUT:
            printf("Out of memory\n");
            break;

        case DIVIDEZERO:
            printf("Divided by zero\n");
            break;

        case MULTIPLEASSIGN:
            printf("Multiple assign\n");
            break;

        case WRONGASSIGN:
            printf("Wrong assign\n");
            break;

        case NUMINTTHEFRONT:
            printf("Number or operator in the front\n");
            break;

        case WRONGEND:
            printf("Wrong end\n");
            break;
    }*/
    exit(0);
}

void get_out(){
int i;
    for(i = 0 ; i < total_statement;i++){
        printCODE(glo_retp[i], print_new_var%8);
        freeTree(glo_retp[i]);
    }

    if(strcmp(registor_storage[0].var_list, "y") == 0 || strcmp(registor_storage[0].var_list, "z") == 0){
        if(strcmp(registor_storage[0].var_list, "y") == 0 &&
           strcmp(registor_storage[1].var_list, "x") != 0 &&
           strcmp(registor_storage[1].var_list, "z") != 0){
                strcpy(registor_storage[1].var_list, registor_storage[0].var_list);
                strcpy(registor_storage[0].var_list, "\0");
                printf("MOV r1 r0\n");
            }

        else if(strcmp(registor_storage[0].var_list, "z") == 0 &&
                strcmp(registor_storage[2].var_list, "x") != 0 &&
                strcmp(registor_storage[2].var_list, "y") != 0){
                strcpy(registor_storage[2].var_list, registor_storage[0].var_list);
                strcpy(registor_storage[0].var_list, "\0");
                printf("MOV r2 r0\n");
            }
        else{
            for(i = 3; strcmp(registor_storage[i].var_list, "x") == 0  ||
                        strcmp(registor_storage[i].var_list, "y") == 0 ||
                        strcmp(registor_storage[i].var_list, "z") == 0 &&
                        i<8 ; i++);
            strcpy(registor_storage[i].var_list, registor_storage[0].var_list);
            strcpy(registor_storage[0].var_list, "\0");
            printf("MOV r%d r0\n",i);
        }
    }
    if(strcmp(registor_storage[1].var_list, "x") == 0 || strcmp(registor_storage[1].var_list, "z") == 0){
        if(strcmp(registor_storage[1].var_list, "x") == 0 &&
           strcmp(registor_storage[0].var_list, "y") != 0 &&
           strcmp(registor_storage[0].var_list, "z") != 0){
                strcpy(registor_storage[0].var_list, registor_storage[1].var_list);
                strcpy(registor_storage[1].var_list, "\0");
                printf("MOV r0 r1\n");
            }
        else if(strcmp(registor_storage[1].var_list, "z") == 0 &&
                strcmp(registor_storage[2].var_list, "x") != 0 &&
                strcmp(registor_storage[2].var_list, "y") != 0){
                strcpy(registor_storage[2].var_list, registor_storage[1].var_list);
                strcpy(registor_storage[1].var_list, "\0");
                printf("MOV r2 r1\n");
            }
        else{
            for(i = 3; strcmp(registor_storage[i].var_list, "x") == 0  ||
                        strcmp(registor_storage[i].var_list, "y") == 0 ||
                        strcmp(registor_storage[i].var_list, "z") == 0 &&
                        i<8 ; i++);
            strcpy(registor_storage[i].var_list, registor_storage[1].var_list);
            strcpy(registor_storage[1].var_list, "\0");
            printf("MOV r%d r1\n",i);
        }
    }
    if(strcmp(registor_storage[2].var_list, "x") == 0 || strcmp(registor_storage[2].var_list, "y") == 0){
        if(strcmp(registor_storage[2].var_list, "x") == 0 &&
           strcmp(registor_storage[0].var_list, "y") != 0 &&
           strcmp(registor_storage[0].var_list, "z") != 0){
            strcpy(registor_storage[0].var_list, registor_storage[2].var_list);
            strcpy(registor_storage[2].var_list, "\0");
            printf("MOV r0 r2\n");
            }
        else if(strcmp(registor_storage[2].var_list, "y") == 0 &&
                strcmp(registor_storage[1].var_list, "x") != 0 &&
                strcmp(registor_storage[1].var_list, "z") != 0){
                strcpy(registor_storage[1].var_list, registor_storage[2].var_list);
                strcpy(registor_storage[2].var_list, "\0");
                printf("MOV r1 r2\n");
            }
        else{
            for(i = 3; strcmp(registor_storage[i].var_list, "x") == 0  ||
                        strcmp(registor_storage[i].var_list, "y") == 0 ||
                        strcmp(registor_storage[i].var_list, "z") == 0 &&
                        i<8 ; i++);
            strcpy(registor_storage[i].var_list, registor_storage[2].var_list);
            printf("MOV r%d r2\n",i);
            strcpy(registor_storage[2].var_list, "\0");
        }
    }

    for(i = 0; strcmp(registor_storage[i].var_list, "x") != 0 && i<8 ; i++);
    if(i>=8) printf("MOV r0 [0]\n");
    else if(i != 0)     printf("MOV r0 r%d\n",i);

    for(i = 0; strcmp(registor_storage[i].var_list, "y") != 0 && i<8 ; i++);
    if(i>=8) printf("MOV r1 [4]\n");
    else if(i != 1)    printf("MOV r1 r%d\n",i);

    for(i = 0; strcmp(registor_storage[i].var_list, "z") != 0 && i<8 ; i++);
    if(i>=8) printf("MOV r2 [8]\n");
    else if(i != 2)    printf("MOV r2 r%d\n",i);

    printf("EXIT 0\n");
        exit(0);

}


void reg_to_mem(int reg){

    if(strcmp(registor_storage[reg].var_list, "x") == 0){
        strcpy(registor_storage[reg].var_list, "\0");
        printf("MOV [0] r%d\n", reg);
        reg_store_flag[reg] = false;
        x_y_z_stored[0] = true;
        strcpy(table2[0].var_list, "x");
        return;
    }
    else if(strcmp(registor_storage[reg].var_list, "y") == 0){
        strcpy(registor_storage[reg].var_list, "\0");
        printf("MOV [4] r%d\n", reg);
        reg_store_flag[reg] = false;
        x_y_z_stored[1] = true;
        strcpy(table2[1].var_list, "y");
        return;
    }
    else if(strcmp(registor_storage[reg].var_list, "z") == 0){
        strcpy(registor_storage[reg].var_list, "\0");
        printf("MOV [8] r%d\n", reg);
        reg_store_flag[reg] = false;
        x_y_z_stored[2] =true;
        strcpy(table2[2].var_list, "z");
        return;
    }
    strcpy(table2[assembly_count].var_list, registor_storage[reg].var_list);
    strcpy(registor_storage[reg].var_list, "\0");
    printf("MOV [%d] r%d\n", 4*assembly_count,reg);
    assembly_count++;
    reg_store_flag[reg] = false;

}



/* clean a tree.*/
void freeTree(BTNode *root){
    if (root!=NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}
