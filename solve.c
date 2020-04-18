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

//�S�ϥΨ쪺*****************************
void printPrefix(BTNode *) ;
int evaluateTree(BTNode *,int) ;
int getval();
void setval(char *, int);
//***************************************


//�ۤv�s�W��***************************************
void printCODE(BTNode *, int); //print assembly code
int get_memory(char *);  //�o�X�@�ӰO����Ŷ�(4n)

typedef struct _var{
    char var_list[MAXLEN];      //�x�s�X�{�L�� variable
    int val;
    bool can_use;
}var;

int count = 3;
var table[65];
bool assign;                    //�P�_�o��statement�O�_�w�gassign�L�F
void get_out();
int total_statement = 0;
BTNode* glo_retp[MAXLEN];

var registor_storage[8];     //�Ȯ��x�s�bregistor���ܼƭ�
bool reg_store_flag[8];      //�T�{�o��registor�O�_���x�s�ܼ�
void reg_to_mem(int);
int print_new_var = 0;       //print���ɭԼW�[���s�ܼ�(�L�צbreg��mem)
int assembly_count = 3;         //count mem�ƶq
bool x_y_z_stored[3];       //x y z ���S���Q�s�i�Lmem
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

        for(i = 0; strcmp(getLexeme(), table[i].var_list) != 0 && i < count ; i++); //��vat_list���S���X�{�L�@�˪��ܼ�

        if(i == count) error(NOTFOUND);//�Ĥ@���X�{�b�k�䪺�ܼ� error
        else retval = table[i].val;

    }
    return retval;
}

void setval(char *str, int val){
    int i;
    for(i = 0; strcmp(str, table[i].var_list) != 0 && i < count ; i++); //��vat_list���S���X�{�L�@�˪��ܼ�

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
                for(int i = 0 ; strcmp(table[i].var_list, root->right->lexeme) != 0;i++);
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
                if(rv == 0) error(DIVIDEZERO);                      //�i��1/(1/100) = 1/0 (int�S��)
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
    //printf("%d,%d,%d,%s\n", lv,rv,root->val,root->lexeme);
    root->val = retval;
    //printf("%d,%d,%d,%s\n", lv,rv,root->val,root->lexeme);
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
        for(i = 0;strcmp(table[i].var_list, lexe) != 0 && i<=count; i++);
        if(table[i].can_use) node->can_use = true;
        else node->can_use = false;
    }
    else node->can_use = true;
    return node;
}

TokenSet getToken(void)
{
    int i;
    char c;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );  // �����ťզr��

    if (isdigit(c)) {//�P�_����@�ӼƦr(�ҥH�̫�|�^��INT) �����o�ǥu�O�b���r�Ű����Ӥw
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i<MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);//�M�Žw�İ�
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+' || c == '-') {//�P�_����@��ADD �� SUB
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    }
    else if (c == '|' || c == '&' || c =='^') {//�P�_����@��OR AND XOR
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ORANDXOR;
    }
    else if (c == '*' || c == '/') {//�P�_����MUL DIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n') {//�P�_�@��⦡����
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=') {    //�P�_���� "="(ASSIGN)
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(') {    //�P�_���� "(" (LPAREN)
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')') {    //�P�_����")" (RPAREN)
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_'){ //�P�_�����ܼ� �ҥH�^��ID (�����o�ǥu�O�b���r�Ű����Ӥw)
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_') {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';//�M�Žw�İ�
        return ID;
    }
    else if (c == EOF) {//�P�_�����Ӵ��굲���F
        return ENDFILE;
    }
    else {
        return UNKNOWN;//�P�_����_�Ǫ��F��
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
        else if (match(ORANDXOR)){// ??????���X��????����error???
            error(NOTNUMID);
        }
        else if (match(LPAREN)) {
            advance();
            retp = expr();
            if (match(RPAREN)) {
                advance();
            }
            else {
                error(MISPAREN);//�����Fexpr���ۦA�����o�S�o�{�A��>�X���D
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

void advance(void)//�Ұ�getToken()�ӧ���e�ݪ����@���جO����token �ýᤩ��lookahead
{
    lookahead = getToken();
}

int match(TokenSet token) //�P�_�o�� token �� lookahead�O�_�@��
{
    if (lookahead == UNKNOWN) advance();
    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}

/* statement := END | expr END */

void statement(int i)//�u�B�z�@��ƾǺ⦡
{
    assign = false;             //��l�� �N���Ĥ@��assign�٨S�X�{
    if (match(ENDFILE)) {       //�̲׵���(�p�G���w��������B�S��exit 1����)
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

        advance();          //get �� ���� �N�A�����@�� �M��A��while�^��

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
                else{//���breg�̭�
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
                else{//���breg�̭�

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
                printf("MOV r%d %d\n",left_reg,node->val);                                        //0.1�����D?
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

            if(i==8)printf("MOV r%d [%d]\n", reg, get_memory(node->lexeme)); //id�bmem �qmem�ɵ����wreg
            else    printf("MOV r%d r%d\n", reg, i);                         //id�breg array �h�����wreg

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

    for(i = 3; strcmp(lex, table2[i].var_list) != 0 && i<=count ; i++); //table������(�O����Ʀr)����print�譱���@��(���ݬd��)
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
