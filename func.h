#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SIZE 4

typedef enum STATEMENT 
{
    INIT,
    IFELSE,
    SIMPLE,
    LOOPS,
    FUNC,
    RETURNN,
    BLOCKK,
    END
} STATEMENT;

typedef struct scope scope;

typedef struct node
{
    char *token;
    char *type;
    struct node *left;
    struct node *middle;
    struct node *right;
} node;

typedef struct symbolTable
{
    char *symbol;
    char *kind;
    char *type;
    scope *parm;
    struct symbolTable *next;
} symbolTable;

typedef struct scope
{
    int number;
    struct symbolTable *STlist;
    struct scope *next;
} scope;

int returnNum = 0;                                                                                  /*כמות החזרות*/
int cMain = 0;                                                                                      /*כמות main*/
scope *stack;                                                                                       /*מחסנית סביבות*/
int Label=1;
int var=0;
int checkAssignments(node *tree, scope *Scope);                                                    /*פונקציה המקבלת הצבה ובודקת האם היא תקינה*/
void printglobalScope(scope *buttomUpScope);                                                        /*הדפסת הסביבה הגלובלית מקבל: stack*/
void addToSTlist(node *tree, node *parm, scope *Scope);                                             /*מוסיף סימבול לסביבה מקבל:עץ של הסימבול וסביבה שלה נוסיף*/
int checkProgram(node *tree, scope *Scope);                                                        /*בודקת Statement מקבלת: עץ וסביבה נוכחית*/
int checkFunc(node *tree, scope *Scope);                                                            /*בודקת אם קיימת פונקציה בעלת אותו שם מקבלת:עץ של פונקציה וסביבה שבה צריך לבדוק*/
symbolTable *searchsymbol(char *search, char *kind, scope *Scope);                                  /*מחפשת שם של סימבול בסביבה מקבלת שם וסביבה*/
int checkInitialization(node *tree, scope *Scope);                                                  /*בודקת אם קיים משתנה בעל אותו שם מקבלת:עץ של הכרזה של משתנה וסביבה שבה צריך לבדוק*/
node *makeNode(char *token, node *left, node *middle, node *right);                                 /*יצירת עץ מקבלת בנים + טוקן*/
scope *makeScope(int number, symbolTable *STlist, scope *next);                                     /*פונקציה שיוצרת סביבה מקבלת מספר רשימת סימבולים ומצביע לסביבה הבאה*/
symbolTable *makeSymbolTable(char *symbol, char *kind, char *type, scope *parm, symbolTable *next); /*פונקציה שיוצרת סימבול מקבלת מחרוזות: סימבול סוג..*/
void printTree(node *tree, int space);                                                              /*הדפסת העץ*/
STATEMENT checkStatement(char *Statement);                                                          /*מחזירה איזה statement*/
void yyerror(const char *msg);                                                                      /*הדפס שגיאה*/
scope *push(int number);                                                                            /*יוצרת סביבה  ומחזירה אותה מקבלת את המספר הסביבה*/
void freeSymbolTable(symbolTable *s);                                                               /*שחרור*/
void pop();                                                                                         /*מחיקת סקוף*/
void popParm();                                                                                     /*שבירת קשר*/
symbolTable *lookUp(char *symbol, char *kind, scope *Scope);                                        /*חיפוש סימבול*/
int chaeckFuncCall(node *sts, scope *Scope);                                                        /*בדיקה האם פונקציה נקראת כמו שצריך*/
char *checkExpr(node *tree, scope *Scope);                                                          /*מקבלת ביטוי ומחזירה את סוגו*/
int checkFuncReturn(node *tree, scope *Scope);                                                     /*בדיקה שבערך המוחזר שווה לערך האמיתי*/

char* freshVar(); /*פונקציה המחזירה רגיסטר חדש*/
char* freshLabel(); /*פונקציה המחזירה לייבל חדש*/

char *GenProgram(node *Head);/*פונקציה המדפיסה את הקוד*/
char* GenFunction(node *tree,int *size,node **Var);/*פונקציה המחזירה את הקוד של הפונקציה*/
char* GenStatement(node *tree,int *size,node **Var);/*פונקציה המחזירה את הקוד של הסטיימנט*/
int getSize(char* type); /*פונקציה המחזירה גודל ביתים של משתנה*/
int checksingel(node *tree);/*בדיקה של השמה פשוטה*/
char* GenSimple(node *tree,int *size,node **Var);  /*בניית קוד עבור פעולה פשוטה*/
char* GenExp(node *tree,int *count,char **VAR,node **Var); /*בניית קוד עבור ביטוי*/
char *GenParm(node *tree,int *size,node **Var);/*החזרת קוד עבור פרמטרים*/
char* GenIF(node *tree,int *size,node **Var);  /*פונקציה המחזירה קוד של שאילתה*/
char* GenLoop(node *tree,int *size,node **Var); /*פונקציה המחזירה קוד של לולאה*/
int findVar(node **vars,char *var);          /*פונקציה המחפש רגיסטר */
node* AddVar(node **vars,char *var);         /*פונקציה המוסיפה רגיסטר*/
int line =0;



symbolTable *lookUp(char *symbol, char *kind, scope *Scope)
{
    scope *run;
    int j;
    int i;
    for (i = Scope->number; i >= 0; i--)
    {
        run = stack;
        j = 0;
        while (j < i)
        {
            run = run->next;
            j++;
        }
        if (searchsymbol(symbol, kind, run) != NULL)
            return searchsymbol(symbol, kind, run);
    }
    return NULL;
}

void printTree(node *tree, int space)
{
    int i;
    if (tree)
    {
        for (i = 0; i < space; i++)
            printf("  ");
        printf("|%s\n", tree->token);
        printTree(tree->left, space + 1);
        printTree(tree->middle, space + 1);
        printTree(tree->right, space + 1);
    }
}

void printglobalScope(scope *buttomUpScope)
{
    int i = 0;
    symbolTable *run;
    symbolTable *run2;
    if (buttomUpScope)
    {
        run = buttomUpScope->STlist;
        printf("%d:\n", buttomUpScope->number);
        while (run != NULL)
        {
            printf("count: %d, symbol: %s, kind: %s, type: %s\n", i, run->symbol, run->kind, run->type);
            if (run->parm != NULL)
            {
                run2 = run->parm->STlist;
                printf("parm:");
                while (run2->next != NULL)
                {
                    printf(" %s %s,", run2->type, run2->symbol);
                    run2 = run2->next;
                }
                printf(" %s %s\n", run2->type, run2->symbol);
            }
            i++;
            run = run->next;
        }
        printglobalScope(buttomUpScope->next);
    }
}

scope *push(int number)
{
    scope *run;
    if (stack == NULL)
    {
        stack = makeScope(0, NULL, NULL);
        return stack;
    }
    run = stack;
    while (run->next != NULL)
        run = run->next;
    run->next = makeScope(number, NULL, NULL);
    return run->next;
}

void pop()
{
    symbolTable *runT1, *runT2;
    scope *runS1, *runS2;
    int flag = 0;
    if (stack == NULL)
        return; /*כאשר אין סביבות*/
    runS1 = stack;
    runS2 = stack;
    while (runS1->next != NULL)
    {
        runS2 = runS1;
        runS1 = runS1->next; /*מציאת סביבה אחרונה*/
    }
    runS2->next = NULL; /*שבירת הקשר בין הסביבות*/
    runT1 = runS1->STlist;
    while (runT1 != NULL)
    {
        runT2 = runT1->next;
        freeSymbolTable(runT1); /*שחרור הסימבול*/
        runT1 = runT2;
    }
    if (runS1 == stack)
    {
        flag = 1;
    }
    free(runS1);
    if (flag == 1)
        stack = NULL;
}

void popParm()
{
    scope *runS1, *runS2;
    if (stack == NULL)
        return; /*כאשר אין סביבות*/
    runS1 = stack;
    while (runS1->next != NULL)
    {
        runS2 = runS1;
        runS1 = runS1->next; /*מציאת סביבה אחרונה*/
    }
    runS2->next = NULL; /*שבירת הקשר בין הסביבות*/
}

void freeSymbolTable(symbolTable *s)
{
    symbolTable *run1;
    symbolTable *run2;
    free(s->symbol);
    free(s->kind); /*שחרור */
    free(s->type);
    if (s->parm != NULL)
    {
        run1 = s->parm->STlist;
        while (run1 != NULL)
        {
            run2 = run1->next; /*שחרור פרמטרים במידה ויש*/
            free(run1);
            run1 = run2;
        }
        free(s->parm);
    }
    free(s);
}

node *makeNode(char *token, node *left, node *middle, node *right)
{
    node *newnode = (node *)malloc(sizeof(node));
    char *newtoken = (char *)malloc(sizeof(token) + 1);
    strcpy(newtoken, token);
    newnode->token = newtoken;
    newnode->left = left;
    newnode->right = right;
    newnode->middle = middle;
    return newnode;
}

symbolTable *makeSymbolTable(char *symbol, char *kind, char *type, scope *parm, symbolTable *next)
{
    symbolTable *newsymboltable = (symbolTable *)malloc(sizeof(symbolTable));
    char *newsymbol = (char *)malloc(sizeof(symbol) + 1);
    char *newkind = (char *)malloc(sizeof(kind) + 1);
    char *newtype = (char *)malloc(sizeof(type) + 1);
    strcpy(newsymbol, symbol);
    strcpy(newkind, kind);
    strcpy(newtype, type);
    newsymboltable->symbol = newsymbol;
    newsymboltable->kind = newkind;
    newsymboltable->type = newtype;
    newsymboltable->next = next;
    newsymboltable->parm = parm;
    return newsymboltable;
}

scope *makeScope(int number, symbolTable *STlist, scope *next)
{
    scope *newscope = (scope *)malloc(sizeof(scope));
    newscope->number = number;
    newscope->STlist = STlist;
    newscope->next = next;
    return newscope;
}

symbolTable *searchsymbol(char *search, char *kind, scope *Scope)
{
    symbolTable *run;
    run = Scope->STlist;
    while (run != NULL)
    {
        if (strcmp(search, run->symbol) == 0 && strcmp(kind, run->kind) == 0)
            break;
        run = run->next;
    }
    return run;
}

STATEMENT checkStatement(char *Statement)
{
    if(Statement==NULL)
        return END;
    if (strcmp(Statement, "init statement") == 0)
        return INIT;
    if (strcmp(Statement, "if/else statement") == 0)
        return IFELSE;
    if (strcmp(Statement, "simple statement") == 0)
        return SIMPLE;
    if (strcmp(Statement, "loops statement") == 0)
        return LOOPS;
    if (strcmp(Statement, "func statement") == 0)
        return FUNC;
    if (strcmp(Statement, "return statement") == 0)
        return RETURNN;
    if (strcmp(Statement, "block statement") == 0)
        return BLOCKK;
    return END;
}

int checkProgram(node *tree, scope *Scope)
{
    node *left;
    node *middle;
    node *temp1;
    node *temp2;
    node *sts;
    char *Fname;
    symbolTable *symbol1 = NULL;
    int check;
    int check2=1;
    if (tree == NULL)
        return 1;
    switch (checkStatement(tree->token))
    {
    case INIT:
        sts = tree->left;
        int x;
        if (strcmp(sts->left->token, "VOID") == 0)
        {
            printf("ERROR: Variable can not be of a type \"VOID\".\n");
            check2= 0;
        }
        if (sts->middle->middle != NULL)                 /*במידה ויש שימוש ב[]*/
            if (strcmp(sts->left->token, "STRING") != 0) /*במידה ויש שימוש ב[] ללא סטרינג*/
            {
                printf("ERROR: The variable \"%s\" can not use [] because it is not a string array.\n", sts->middle->left->token);
                check2= 0;
            }
            else
            {
                if (checkInitialization(sts, Scope) == 1) /*בדיקה האם קיים כבר משתנה בשם זהה בסביבה*/
                    addToSTlist(sts, NULL, Scope);        /*הוספת משתנה לסביבה*/
                                                          /*לבדוק את כל האופציות*/
                else check2=0;
            }
        else
        {
            if (checkInitialization(sts, Scope) == 1) /*בדיקה האם קיים כבר משתנה בשם זהה בסביבה*/
            {
                    addToSTlist(sts, NULL, Scope);        /*הוספת משתנה לסביבה*/
            }
            else
                {
                    left = (node *)malloc(sizeof(node));
                    left->token = (char *)malloc(sizeof(sts->left->token)); /*בניית עץ עבור סוג המשתנה*/
                    strcpy(left->token, sts->left->token);
                    printf("ERROR: The variable %s has been defined more than once.\n", sts->middle->token);
                    check2 =  0;
                }
        }
        if (sts->right != NULL) /*מצב של הכרזת משתנים מרובה*/
        {
            left = (node *)malloc(sizeof(node));
            left->token = (char *)malloc(sizeof(sts->left->token)); /*בניית עץ עבור סוג המשתנה*/
            strcpy(left->token, sts->left->token);
            middle = (node *)malloc(sizeof(node));
            sts = sts->right;
            do
            {
                middle->token = (char *)malloc(sizeof(sts->left->token)); /*בניית עץ עבור שם המשתנה*/
                strcpy(middle->token, sts->left->token);
                if(sts->left->left != NULL)
                {
                    middle->left=makeNode(sts->left->left->token,NULL,NULL,NULL);
                }
                temp1 = makeNode("init", left, middle, NULL);                        /*בניית עץ עבור ההכרזה*/
                if (sts->left->left != NULL) /*במידה ויש שימוש ב[] ללא סטרינג*/
                {
                    if(strcmp(left->token,"STRING")!=0)
                    {
                        printf("ERROR: The variable \"%s\" can not use [] because it is not a string array.\n", sts->left->left->token);
                        check2= 0;
                    }
                }
                if (checkInitialization(temp1, Scope) == 1)
                    addToSTlist(temp1, NULL, Scope);
                else
                {
                    printf("ERROR: The variable %s has been defined more than once.\n", temp1->middle->token);
                    check2= 0;
                }
                if(middle->left!=NULL)
                {
                    free(middle->left->token);
                }
                free(middle->token);
                free(temp1->token);
                sts = sts->middle;
            } while (sts != NULL);
            free(left->token);
            free(left);
            free(middle);
        }
        break;
    case IFELSE:
        sts = tree->left;
        if (checkExpr(sts->left, Scope) != NULL && strcmp(checkExpr(sts->left, Scope), "BOOLEAN") != 0)
        {
            printf("ERROR: A non boolean expression as a condition of IF statement is illegal.\n");
            check2= 0;
        }
        else
        {
            if (strcmp(sts->middle->token, "BLOCK") == 0) /*מקרה של בלוק*/
            {
                check2 = checkProgram(sts->middle->left, push(Scope->number + 1));
                pop();
            }
            else
                check2 = checkProgram(sts->middle, Scope); /*מקרה של פעולה אחת*/
            if (sts->right != NULL)
            {
                if (strcmp(sts->right->left->token, "BLOCK") == 0) /*מקרה של בלוק*/
                {
                    check2 = checkProgram(sts->right->left->left, push(Scope->number + 1));
                    pop();
                }
                else
                    check2 = checkProgram(sts->right->left, Scope); /*מקרה של פעולה אחת*/
            }
        }
        break;
    case SIMPLE:
        sts = tree->left;
        if (strcmp(sts->token, "callFunc") == 0) /*קריאה לפונקציה*/
            check2 = chaeckFuncCall(sts, Scope);
        if (strcmp(sts->token, "=") == 0)
        {
            check2 =  checkAssignments(sts, Scope);
        }
        break;
    case LOOPS:
        sts = tree->left->left;
        if (strcmp(sts->token, "WHILE") == 0)
        {
            if (sts->left != NULL && checkExpr(sts->left, Scope) != NULL)
            {

                if (strcmp(checkExpr(sts->left, Scope), "BOOLEAN") != 0)
                {
                    printf("ERROR: A non boolean expression as a condition of WHILE is illegal.\n"); /*חלק 2*/
                    check2 = 0;
                }
                if (strcmp(sts->middle->token, "BLOCK") == 0) /*מקרה של בלוק*/
                {
                    check2 = checkProgram(sts->middle->left, push(Scope->number + 1));
                    pop();
                }
                else
                    check2 = checkProgram(sts->middle, Scope); /*מקרה של פעולה אחת*/
            }
        }
        if (strcmp(sts->token, "DO") == 0)
        {
            check2 = checkProgram(sts->left->left, push(Scope->number + 1));
            pop();
            if (sts->middle->left != NULL && checkExpr(sts->middle->left, Scope) != NULL)
            {

                if (strcmp(checkExpr(sts->middle->left, Scope), "BOOLEAN") != 0)
                {
                    printf("ERROR:A non boolean expression as a condition of DO WHILE is illegal.\n"); /*חלק 2*/
                    check2 =  0;
                }
            }
        }
        if (strcmp(sts->token, "FOR") == 0)
        {
            temp1 = sts->left;
            if (temp1->left != NULL)
            {
                temp2 = makeNode("simple statement", temp1->left, NULL, NULL);
                check2 = checkProgram(temp2, Scope);
            }
            if (temp1->middle != NULL && checkExpr(temp1->middle, Scope) != NULL)
            {
                if (strcmp(checkExpr(temp1->middle, Scope), "BOOLEAN") != 0)
                {
                    printf("ERROR:A non boolean expression as a condition of FOR is illegal.\n"); /*חלק 2*/
                    check2 = 0;
                }
            }
            if (temp1->right != NULL)
            {
                temp2 = makeNode("simple statement", temp1->right, NULL, NULL);
                check2 = checkProgram(temp2, Scope);
            }

            if (strcmp(sts->middle->token, "BLOCK") == 0) /*מקרה של בלוק*/
            {
                check2 = checkProgram(sts->middle->left, push(Scope->number + 1));
                pop();
            }
            else
                check2 = checkProgram(sts->middle, Scope); /*מקרה של פעולה אחת*/
        }
        break;
    case FUNC:
        sts = tree->left;
        if (strcmp(sts->token, "MAIN") == 0) /*בדיקה שהתוכנית הראשית לא הוגדרה*/
        {
            cMain += 1;
            if (cMain == 1) /*מקרה של פונקציה main*/
                addToSTlist(sts->left, NULL, Scope);
            else
            {
                printf("ERROR: The function main should be defined just once.\n");
                check2 =  0;
            }
            check2 = checkProgram(sts->middle->left, push(Scope->number + 1)); /*בדיקת הסביבה של main*/
            pop();
        }
        else
        {
            if (checkFunc(sts->left, Scope) == 1) /*בדיקה אם הפונקציה הוגדרה בעבר*/
                addToSTlist(sts->left, sts->left->right, Scope);
            else
            {
                printf("ERROR: The function %s has been defined more than once.\n", sts->left->middle->token);
                check2 = 0;
            }
            if (sts->left->right != NULL) /*בדיקה אם יש פרמטרים*/
            {
                if (strcmp(sts->left->left->token, "VOID") != 0)
                {
                    returnNum += 1;
                    check = returnNum;
                }
                check2 = checkProgram(sts->middle->left, push(Scope->number + 2));
                pop();
                popParm();
                if (strcmp(sts->left->left->token, "VOID") != 0 && check2==1)
                    if (check == returnNum)
                    {
                        printf("ERROR: The function \"%s\" must return a value!\n", sts->left->middle->token);
                        check2 =  0;
                    }
            }
            else
            {
                if (strcmp(sts->left->left->token, "VOID") != 0)
                {
                    returnNum += 1;
                    check = returnNum;
                }
                check2 = checkProgram(sts->middle->left, push(Scope->number + 1));
                pop();
                if (strcmp(sts->left->left->token, "VOID") != 0 && check2==1)
                    if (check == returnNum)
                    {
                        printf("ERROR: The function \"%s\" must return a value!\n", sts->left->middle->token);
                        check2 = 0;
                    }
            }
        }
        break;
    case RETURNN:
        sts = tree->left;
        check2 = checkFuncReturn(sts->left, Scope);
        returnNum -= 1;
        break;
    case BLOCKK:
        check2 = checkProgram(tree->left->left, push(Scope->number + 1)); /*בדיקת סביבה של הבלוק*/
        pop();
        break;
    case END:
        break;
    }
    if (check2==0)
        return check2;
    check2 = checkProgram(tree->middle, Scope);
    return check2;
}

int checkAssignments(node *tree, scope *Scope)
{
    int chang = 0;
    symbolTable *left;
    node *temp;
    int point=0;
    char *check;
    if (strcmp(tree->left->token, "array") == 0)
        left = lookUp(tree->left->left->token, "var", Scope);
    else
        {
            if (strcmp(tree->left->token, "pointer") == 0)
            {
                left = lookUp(tree->left->left->token, "var", Scope);
                point=1;
                if(left)
                {
                    if (strcmp(left->type, "CHARP") != 0 && strcmp(left->type, "INTP") != 0)
                        {
                            printf("ERROR: Can not apply ^ to a non pointer expression.\n");
                            return 0;
                        }
                }                    
            }
            else left = lookUp(tree->left->token, "var", Scope);
        }

    if (left == NULL)
    {
        if (strcmp(tree->left->token, "array") != 0)
            printf("ERROR: The variable \"%s\" has been used without getting defined.\n", tree->left->token);
        else
            printf("ERROR: The variable \"%s\" has been used without getting defined.\n", tree->left->left->token);
        return 0;
    }
    else
    {
        tree->left->type =  (char *)malloc(sizeof(left->type));
        strcpy(tree->left->type,left->type);  /***/
        if (checkExpr(tree->middle, Scope) != NULL)  /*בדיקת צד ימין*/
        {
            if (tree->left->middle != NULL) /*מקרה של מערך*/
            {
                if (strcmp(left->type, "STRING") != 0) /*אם יש שימוש של [] והמשתנה אינו סטרינג*/
                {
                    printf("ERROR: The variable \"%s\" can not use [] because it is not a string array.\n", tree->left->left->token);
                    return 0;
                }
                else
                {
                    if (strcmp(tree->middle->token, "pointer") == 0) /*כשאר יש שימוש בפיונטר*/
                    {
                        if (strcmp(checkExpr(tree->middle, Scope), "CHARP") != 0)
                        {
                            printf("ERROR: Can not apply ^ to a non pointer expression.\n");
                            return 0;
                        }
                        tree->middle->token = "pointer";
                    }
                    else if (strcmp(checkExpr(tree->middle, Scope), "CHAR") != 0 && !(strcmp(checkExpr(tree->middle, Scope), "STRING") == 0 && tree->middle->left->left != NULL)) /*כאשר מנסים להציב לא char*/
                    {
                        printf("ERROR: There is an illegal assignment of different types(%s <-- %s).\n", "CAHRp", checkExpr(tree->middle, Scope));
                        return 0;
                    }
                }
            }
            else if (strcmp(left->type, "CHARP") == 0 && point ==0) /*מקרה של charp*/
            {
                if (strcmp(tree->middle->token, "addres") == 0)
                {
                    if (strcmp(checkExpr(tree->middle, Scope), "CHAR") != 0)
                    {
                        printf("ERROR: There is an illegal assignment of different types(%s <-- &%s).\n", "CAHRP", checkExpr(tree->middle, Scope));
                        return 0;
                    }
                    tree->middle->token = "addres";
                }
                else
                {
                    if (strcmp(checkExpr(tree->middle, Scope), "NULL") != 0 && strcmp(checkExpr(tree->middle, Scope), "CAHR") != 0)
                    {
                        printf("ERROR: There is an illegal assignment of different types(%s <-- %s).\n", "CAHRP", checkExpr(tree->middle, Scope));
                        return 0;
                    }
                }
            }
            else if (strcmp(left->type, "INTP") == 0 && point ==0 ) /*מקרה של intp*/
                {
                    if (strcmp(tree->middle->token, "addres") == 0)
                    {
                        tree->middle->token = "identifier";
                        if (strcmp(checkExpr(tree->middle, Scope), "INT") != 0)
                        {
                            printf("ERROR: There is an illegal assignment of different types(%s <-- &%s).\n", "INTP", checkExpr(tree->middle, Scope));
                            return 0;
                        }
                        tree->middle->token = "addres";
                    }
                    else
                    {
                        if (strcmp(checkExpr(tree->middle, Scope), "NULL") != 0 && strcmp(checkExpr(tree->middle, Scope), "INTP") != 0)
                        {
                            printf("ERROR: There is an illegal assignment of different types(%s <-- %s).\n", "INTP", checkExpr(tree->middle, Scope));
                            return 0;
                        }
                    }
                }
                else
                {
                    if (strcmp(tree->middle->token, "pointer") == 0)
                    {
                        if (checkExpr(tree->middle, Scope)==NULL)
                        {
                            return 0;
                        }
                        else if (strcmp(checkExpr(tree->middle, Scope), "CHARP") == 0)
                        {
                            if (strcmp(left->type, "CHAR") != 0)
                            {
                                printf("ERROR: There is an illegal assignment of different types (%s <-- %s).\n", left->type, "CHAR");
                                return 0;
                            }
                        }
                        else if (strcmp(checkExpr(tree->middle, Scope), "INTP") == 0)
                        {
                            {
                                printf("ERROR: There is an illegal assignment of different types (%s <-- %s).\n", left->type, "INT");
                                return 0;
                            }
                        }
                    }
                    else
                        {
                        if (strcmp(tree->middle->token, "addres") == 0)
                        {
                            printf("ERROR: Can not apply & to a non char nor integer expression.\n");
                            return 0;
                        }
                        if(point==1) /*מקרה של פוינטר*/
                        {
                            if (strcmp(left->type,"INTP") == 0)
                                check="INT";
                            else check="CHAR";
                            if (strcmp(check, checkExpr(tree->middle, Scope)) != 0)
                            {
                                printf("ERROR: There is an illegal assignment of different types (%s <-- %s).\n", check, checkExpr(tree->middle, Scope));
                                return 0;
                                }
                        }
                        else if (strcmp(left->type, checkExpr(tree->middle, Scope)) != 0)
                            {
                                printf("ERROR: There is an illegal assignment of different types (%s <-- %s).\n", left->type, checkExpr(tree->middle, Scope));
                                return 0;
                            }
                    }

                }
            
        }
        else return 0;
    }
    return 1;

}

int checkFunc(node *tree, scope *Scope)
{
    symbolTable *temp = searchsymbol(tree->middle->token, "func", Scope);
    if (temp == NULL || strcmp(temp->kind, "func") != 0)
        return 1;
    return 0;
}

int checkInitialization(node *tree, scope *Scope)
{
    symbolTable *temp;
    if (tree->middle->left != NULL) /*במקרה של מערך*/
    {
        temp = searchsymbol(tree->middle->left->token, "var", Scope);
    }
    else
    {
        temp = searchsymbol(tree->middle->token, "var", Scope);
    }
    if (temp == NULL)
        return 1;
    return 0;
}

void addToSTlist(node *tree, node *parm, scope *Scope)
{
    symbolTable *temp = NULL;
    symbolTable *run = NULL;
    symbolTable *Flist = NULL;
    symbolTable *N1list = NULL;
    symbolTable *N2list = NULL;
    int length;
    scope *Parmlist;
    length = 0;
    Parmlist = NULL;
    char *kind;
    if (strcmp("init", tree->token) == 0)
    {
        kind = (char *)malloc(sizeof(4)); /*הגדרת סוג למשתנה*/
        strcpy(kind, "var");
    }
    if (strcmp("FUNCINFO", tree->token) == 0 || strcmp("MAININFO", tree->token) == 0)
    {
        kind = (char *)malloc(sizeof(5)); /*הגדרת סוג לפונקציה*/
        strcpy(kind, "func");
    }
    if (parm != NULL) /*אם יש פרמטרים*/
    {
        Flist = makeSymbolTable(parm->middle->token, "var", parm->left->token, NULL, NULL);
        if (parm->right != NULL)
        {
            N2list = Flist;
            do
            {
                parm = parm->right;
                N1list = makeSymbolTable(parm->middle->token, "var", parm->left->token, NULL, NULL); /*בניית סימבולים עבור הפרמטרים*/
                N2list->next = N1list;
                N2list = N1list;
            } while (parm->right != NULL);
        }
        Parmlist = push(Scope->number + 1); /*יצירת סקופ המכיל את הפרטנרים*/
        Parmlist->STlist = Flist;
    }
    if (Scope->STlist == NULL)
    {
        if (tree->middle->left != NULL) /*מקרה של מערך*/
        {
            Scope->STlist = makeSymbolTable(tree->middle->left->token, kind, tree->left->token, Parmlist, NULL); /*מקרה שזהו הסימבול הראשון בסקופ*/
        }
        else
            Scope->STlist = makeSymbolTable(tree->middle->token, kind, tree->left->token, Parmlist, NULL); /*מקרה שזהו הסימבול הראשון בסקופ*/
        return;
    }
    run = Scope->STlist;
    while (run->next != NULL)
        run = run->next;
    if (tree->middle->left != NULL) /*מקרה של מערך*/
    {

        temp = makeSymbolTable(tree->middle->left->token, kind, tree->left->token, Parmlist, NULL); /*מקרה שזהו הסימבול הראשון בסקופ*/
    }
    else
        temp = makeSymbolTable(tree->middle->token, kind, tree->left->token, Parmlist, NULL); /*מקרה שזהו הסימבול הראשון בסקופ*/
    run->next = temp;
    free(kind);
}

int chaeckFuncCall(node *sts, scope *Scope)
{
    char *Fname;
    symbolTable *symbol1 = NULL;
    int check = 1;
    char *type = NULL;
    if (lookUp(sts->left->token, "func", Scope) == NULL) /*בדיקה האם הפונקציה הוגדרה*/
    {
        printf("ERROR: The function \"%s\" is not defined.\n", sts->left->token);
        check = 0;
    }
    else
    {
        sts->left->type =  (char *)malloc(sizeof(lookUp(sts->left->token, "func", Scope)->type));
        strcpy(sts->left->type,lookUp(sts->left->token, "func", Scope)->type);  /***/
        Fname = sts->left->token;
        if (sts->middle != NULL) /*אם יש לפונקציה פרמטרים*/
        {
            sts = sts->middle;
            symbol1 = lookUp(Fname, "func", Scope);
            if (symbol1->parm != NULL)
                symbol1 = symbol1->parm->STlist; /*שליפת הפרמטרים מהטבלה*/
            else
            {
                printf("ERROR: There is an illegal function call of \"%s\" with big amount of arguments.\n", Fname); /*מקרה שבו אין פרמטרים בטבלה*/
                check = 0;
            }
            while (symbol1 != NULL && check == 1) /*עד שבדקנו את כל הפרמטרים או שמצאנו שגיאה*/
            {
                if (sts == NULL)
                {
                    printf("ERROR: There is an illegal function call of \"%s\" with small amount of arguments.\n", Fname); /*כמות קטנה של מידי של פרמטרים*/
                    check = 0;
                }
                else
                {
                    type = checkExpr(sts->left, Scope);
                    if (type == NULL)
                        check = 0;
                    else if (strcmp(type, symbol1->type) != 0)
                    {
                        check = 0; /*פרמטרים מסוג שונה*/
                        printf("ERROR: There is an illegal function call of \"%s\" with different arguments types (%s != %s).\n", Fname, type, symbol1->type);
                    }
                    else
                    {
                        sts = sts->middle;
                        symbol1 = symbol1->next; /*בדיקת הפרמטר הבא*/
                    }
                }
            }
            if (sts != NULL && check == 1) /*כאשר יש יותר מידי פרמטרים*/
            {
                printf("ERROR: There is an illegal function call of \"%s\" with big amount of arguments.\n", Fname);
                check = 0;
            }
        }
        else
        {
            if (lookUp(Fname, "func", Scope)->parm != NULL) /*כאשר הקריאה אין פרמטרים אבל בהצהרה יש*/
            {
                printf("ERROR: There is an illegal function call of \"%s\" with small amount of arguments.\n", Fname);
                check = 0;
            }
        }
    }
    return check;
}

char *checkExpr(node *tree, scope *Scope) /*פונקציה המקבלת ביטוי ומחזירה את סוגו*/
{
    node *left;
    char* temp;
    int str=0;
    if (tree == NULL)
        return NULL;
    if (strcmp(tree->token, "!") == 0)
    {
        if (checkExpr(tree->left, Scope) == NULL)
            return NULL;
        tree->left->type =  (char *)malloc(sizeof("BOOLEAN"));
        strcpy(tree->left->type,"BOOLEAN");  /***/
        if (strcmp(checkExpr(tree->left, Scope), "BOOLEAN") != 0) /*שימוש ב! עבור משתנה לא בוליאני*/
        {
            printf("ERROR: Can not apply ! to a non boolean expression.\n");
            return NULL;
        }
        else
            return "BOOLEAN";
    }
    if (strcmp(tree->token, "identifier") == 0)
    {
        char *check;
        if(tree->left->left!=NULL)
            check=tree->left->left->token;
        else check=tree->left->token;
        if (lookUp(check, "var", Scope) == NULL)
        {
            printf("ERROR: The var \"%s\" is not defined.\n", tree->left->token); /*פרמטר לא מוגדר*/
            return NULL;
        }
        else
        {
            tree->type =  (char *)malloc(sizeof(lookUp(check, "var", Scope)->type));
            strcpy(tree->type,lookUp(check, "var", Scope)->type);  /***/
            return lookUp(check, "var", Scope)->type;
        }
    }
    if (strcmp(tree->token, "pointer") == 0)
    {
        if (lookUp(tree->left->left->token, "var", Scope) == NULL)
        {
            printf("ERROR: The var \"%s\" is not defined.\n", tree->left->token); /*פרמטר לא מוגדר*/
            return NULL;
        }
        else
        {
            if(strcmp(lookUp(tree->left->left->token, "var", Scope)->type,"INTP")!=0 && strcmp(lookUp(tree->left->left->token, "var", Scope)->type,"CHARP")!=0)
               {
                    printf("**ERROR: Can not apply ^ to a non pointer expression.\n");
                    return NULL;
               } 
            if(strcmp(lookUp(tree->left->left->token, "var", Scope)->type,"INTP")==0)
            {
                tree->left->type = (char *)malloc(sizeof("INTP"));
                strcpy(tree->left->type,"INTP");  /***/
                tree->type = (char *)malloc(sizeof("INTP"));
                strcpy(tree->type,"INTP");  /***/
                return "INT";
            }
            else  
            {
                tree->left->type = (char *)malloc(sizeof("INTP"));
                strcpy(tree->left->type,"INTP");  /***/
                tree->type = (char *)malloc(sizeof("INTP"));
                strcpy(tree->type,"INTP");  /***/
                return "CHAR";
            }
        }
    }
    if (strcmp(tree->token, "addres") == 0) 
    {
        if(strcmp(tree->left->left->token,"array")==0)
        {
            temp=tree->left->left->left->token;
            str=1;
        }
        else temp=tree->left->left->token;
        if (lookUp(temp, "var", Scope) == NULL)
        {
            printf("ERROR: The var \"%s\" is not defined.\n", tree->left->token); /*פרמטר לא מוגדר*/
            return NULL;
        }
        else
        {
            if(str==1)
            {
                if(strcmp(lookUp(temp, "var", Scope)->type,"STRING")==0)
                {
                    tree->type = (char *)malloc(sizeof("CHAR"));
                    strcpy(tree->type,"CHAR");  /***/
                    tree->left->left->type = (char *)malloc(sizeof("CHAR"));
                    strcpy(tree->left->left->type,"CHAR");  /***/

                    return "CHAR";
                }
                printf("ERROR: The variable \"%s\" can not use [] because it is not a string array.\n", temp);
                return NULL;
            }
            if(strcmp(lookUp(temp, "var", Scope)->type,"INT")==0)
            {
                tree->left->type = (char *)malloc(sizeof("INT"));
                strcpy(tree->left->type,"INT");  /***/
                tree->left->left->type = (char *)malloc(sizeof("INT"));
                strcpy(tree->left->left->type,"INT");  /***/
                return "INTP";
            }
            if(strcmp(lookUp(temp, "var", Scope)->type,"CHAR")==0)
            {
                tree->left->type = (char *)malloc(sizeof("CHAR"));
                strcpy(tree->left->type,"CHAR");  /***/
                return "CHARP"; 
            }
            
            printf("ERROR: The variable \"%s\" can not use '&' because it is not 'CHAR' or 'INT' or 'string[index]'.\n", temp);
            return NULL;
        }
    }
    if (((strcmp(tree->token, "+") == 0) || (strcmp(tree->token, "-") == 0)) && (tree->middle == NULL)) //plus & minus
    {
        if (checkExpr(tree->left, Scope) == NULL)
        {
            return NULL;
        }
        if (strcmp(checkExpr(tree->left, Scope), "INT") == 0)
        {
                tree->left->type = (char *)malloc(sizeof("INT"));
                strcpy(tree->left->type,"INT");  /***/
                return "INT"; 
        }
        else
        {
            printf("ERROR: Can not apply the operator \'%s\' on expressions that are not integers.\n", tree->token);
            return NULL;
        }
    }
    if (strcmp(tree->token, "FuncExpr") == 0)
    {
        if (chaeckFuncCall(tree, Scope) == 1)
        {
            tree->left->type = (char *)malloc(sizeof(lookUp(tree->left->token, "func", Scope)->type));
            strcpy(tree->left->type,lookUp(tree->left->token, "func", Scope)->type);  /***/
            return (lookUp(tree->left->token, "func", Scope)->type);
        }
        else
            return NULL;
    }
    if (strcmp(tree->token, "length/absolute") == 0)
    {
        if (lookUp(tree->left->left->token, "var", Scope) == NULL)
        {
            printf("ERROR: The var \"%s\" is not defined.\n", tree->left->left->token); /*פרמטר לא מוגדר*/
            return NULL;
        }
        else
        {
            if (strcmp(lookUp(tree->left->left->token, "var", Scope)->type, "INT") == 0 || strcmp(lookUp(tree->left->left->token, "var", Scope)->type, "STRING") == 0)
            {
                tree->left->type = (char *)malloc(sizeof("INT"));
                strcpy(tree->left->type,"INT");  /***/
                tree->type = (char *)malloc(sizeof("INT"));
                strcpy(tree->type,"INT");  /***/
                return "INT"; 
            }
            else
            {
                printf("ERROR: Can not apply Absolute(\'| |\') to a non integer nor string expression.\n");
                return NULL;
            }
        }
    }
    if (strcmp(tree->token, "NUM") == 0)
    {
        tree->type = (char *)malloc(sizeof("INT"));
        strcpy(tree->type,"INT");  /***/
        return "INT"; 
    }
    if (strcmp(tree->token, "boolean") == 0)
    {
        tree->type = (char *)malloc(sizeof("BOOLEAN"));
        strcpy(tree->type,"BOOLEAN");  /***/
        return "BOOLEAN";
    }
    if (strcmp(tree->token, "CHAR") == 0)
    {
        tree->type = (char *)malloc(sizeof("CHAR"));
        strcpy(tree->type,"CHAR");  /***/
        return "CHAR";
    }
    if (strcmp(tree->token, "string") == 0)
    {
        tree->type = (char *)malloc(sizeof("string"));
        strcpy(tree->type,"string");  /***/
        return "STRING";
    }
    if (strcmp(tree->token, "NULL") == 0)
    {
        return "NULL";
    }
    if (strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0)
    {
        if (checkExpr(tree->left, Scope) == NULL)
        {
            return NULL;
        }
        if (checkExpr(tree->middle, Scope) == NULL)
        {
            return NULL;
        }
        if (strcmp(checkExpr(tree->left, Scope), checkExpr(tree->middle, Scope)) != 0)
        {
            printf("ERROR: Can not apply the operator \'%s\' on expressions that are not pairs of integers, booleans, chars, or pointers (%s != %s).\n", tree->token,checkExpr(tree->left, Scope), checkExpr(tree->middle, Scope));
            return NULL;
        }
        return "BOOLEAN";
    }
    if (strcmp(tree->token, ">") == 0 || strcmp(tree->token, ">=") == 0 || strcmp(tree->token, "<") == 0 || strcmp(tree->token, "<=") == 0)
    {
        if (checkExpr(tree->left, Scope) == NULL)
        {
            return NULL;
        }
        if (checkExpr(tree->middle, Scope) == NULL)
        {
            return NULL;
        }

        if (strcmp(checkExpr(tree->left, Scope), "INT") != 0 || strcmp(checkExpr(tree->middle, Scope), "INT") != 0)
        {
            printf("ERROR: Can not apply the operator \'%s\' on expressions that are not integers.\n", tree->token);
            return NULL;
        }
        else
            return "BOOLEAN";
    }
    if (strcmp(tree->token, "OR") == 0 || strcmp(tree->token, "AND") == 0)
    {
        if (checkExpr(tree->left, Scope) == NULL)
        {
            return NULL;
        }
        if (checkExpr(tree->middle, Scope) == NULL)
            return NULL;
        if (strcmp(checkExpr(tree->left, Scope), "BOOLEAN") != 0 || strcmp(checkExpr(tree->middle, Scope), "BOOLEAN") != 0)
        {
            printf("ERROR: Can not apply the operator \'%s\' on expressions that are not booleans.\n", tree->token);
            return NULL;
        }
        else
            return "BOOLEAN";
    }
    if (((strcmp(tree->token, "*") == 0) || (strcmp(tree->token, "/") == 0) || (strcmp(tree->token, "+") == 0) || (strcmp(tree->token, "-") == 0)) && (tree->middle != NULL)) //mul & div
    {
        if (checkExpr(tree->left, Scope) == NULL)
        {
            return NULL;
        }
        if (checkExpr(tree->middle, Scope) == NULL)
        {
            return NULL;
        }
        if ((strcmp(checkExpr(tree->left, Scope), "INT") == 0) && (strcmp(checkExpr(tree->middle, Scope), "INT") == 0))
        {
            return "INT";
        }
        else
        {
            printf("ERROR: Can not apply the operator \'%s\' on expressions that are not int.\n", tree->token);
            return NULL;
        }
    }
}

int checkFuncReturn(node *tree, scope *Scope) /*פונקציה המקבלת ערך מוחזר ובודקת האם ניתן להחזירו*/
{
    symbolTable *tempsymbol = NULL;
    scope *p = stack;
    char *returnType = NULL;
    while (p->next != Scope) /*חיפוש סקופ המכיל את הפונקציה*/
    {
        tempsymbol = p->STlist;
        while (tempsymbol->next != NULL) /*מציאת הסימבול של הפונקציה*/
        {
            tempsymbol = tempsymbol->next;
        }
        if (tempsymbol->parm != NULL && p->next->next == Scope) /*אם יש פרמטרים בדיקת הסקופ הפנימי*/
            break;
        p = p->next;
    }
    tempsymbol = p->STlist;
    while (tempsymbol->next != NULL) /*מציאת הסימבול של הפונקציה*/
    {
        tempsymbol = tempsymbol->next;
    }
    if (strcmp(tempsymbol->type, "VOID") == 0)
    {
        if (tree == NULL)
            return 1;
        else
        {
            printf("Function \"%s\" can not return a value!\n", tempsymbol->symbol);
            return 0;
        }
    }
    returnType = checkExpr(tree, Scope); /*מציאת סוג הערך המוחזר*/
    if (returnType != NULL)
    {
        if (strcmp(returnType, "STRING") == 0)
        {
            printf("ERROR: The function can not return string!\n");
            return 0; // returning a string is ILLEGAL!
        }

        if (strcmp(returnType, "NULL") == 0)
        {
            if ((strcmp(tempsymbol->type, "CHARP") != 0 && strcmp(tempsymbol->type, "INTP") != 0))
                printf("ERROR: The returned value not suit the function decleration!\n");
            return 0;
        }

        if (strcmp(returnType, tempsymbol->type) != 0)
        {
            printf("ERROR: The return type of function \"%s\" is different from the actual returned value type \"%s\".\n", tempsymbol->type, returnType);
            return 0;
        }
    }
    return 1;
}

char* freshVar() 
{
	char *a;
	asprintf(&a,"_t%d",var++);
	return a;
}
char* freshLabel() 
{
	char *a;
	asprintf(&a,"L%d",line);
	return a;
}

int getSize(char* type) /*פונקציה המחזירה גודל ביתים של משתנה*/
{
    if(strcmp(type,"CHAR")==0)
        return 1;
    if(strcmp(type,"INT")==0)
        return 4;
    if(strcmp(type,"BOOLEAN")==0)
        return 1;
    if(strcmp(type,"CHARP")==0)
        return 4;
    if(strcmp(type,"INTP")==0)
        return 4;
    if(strcmp(type,"STRING")==0)
        return 4;

}
node* AddVar(node **vars,char *var)
{
    node *run;
    if(*vars==NULL)
    {
            *vars = makeNode(var,NULL,NULL,NULL);
    }
    else
        {
            run=*vars;
            while(run->left!=NULL)
                run=run->left;
            run->left=makeNode(var,NULL,NULL,NULL);
        }

    return *vars;
}
int findVar(node **vars,char *var)
{
    node *run;
    run=*vars;
    while(run!=NULL)
    {
        if(strcmp(run->token,var)==0)
            return 1;
        run=run->left;
    }
    return 0;      
}
char* GenProgram(node *Head)/*  פונקציה המחזירה את הקוד של התוכנית*/
{
    int size=0;
    char *Code="";
    char *temp;
    node **vars=NULL;
    node *functionRun=Head; 
    node *function;
    do{
        vars= (node **)malloc(sizeof(node*));
        *vars=NULL;
        function=functionRun->left;
        if(strcmp(function->token,"MAIN")==0)
        {
            line=line+2;
            temp=GenFunction(function->middle->left,&size,vars);
            line=line+2;
            asprintf(&Code,"%s%s:\n\t%s%d\n%s\t%s\n\n",Code,"MAIN","BeginFunc ",size,temp,"EndFunc");
            size=0;
        }
        else
        {
            line=line+2;
            temp=GenFunction(function->middle->left,&size,vars);
            line=line+2;
            asprintf(&Code,"%s%s:\n\t%s%d\n%s\t%s\n\n", Code, function->left->middle->token,"BeginFunc ",size,temp,"EndFunc");
            size=0;
        }
        functionRun=functionRun->middle;
        }while(functionRun!=NULL);
    return Code;

}

char* GenFunction(node *tree,int *size,node **vars)/*פונקציה המחזירה את הקוד של הפונקציה*/
{
    char *CodeFunc="";
    char* stsCose;
    node *sts=tree;
    do{                                        /*ריצה על כל הסטיימנט*/
        stsCose=GenStatement(sts,size,vars);
        if(stsCose)
        {
            asprintf(&CodeFunc,"%s%s", CodeFunc, stsCose);
        }
        if(sts!=NULL)
            sts = sts->middle;
        }while(sts!=NULL);
    return CodeFunc;
}

char* GenStatement(node *tree,int *size,node **vars)/*פונקציה המחזירה את הקוד של הסטיימנט*/
{
    char *CodeSts;
    char *VAR="";
    char *CoseExp;
    node *save;
    if(tree==NULL)
        return "";
    switch(checkStatement(tree->token))   /*מציאת הסטיימנט */
    {
        case SIMPLE:
            CodeSts=GenSimple(tree->left,size,vars); /*מציאת הקוד והחזרתו*/
            return CodeSts;
            break;
        case RETURNN:
            if(tree->left->left!=NULL)       /*שימוש בהחזרה בפונקציית void*/
            {
                CoseExp=GenExp(tree->left->left,size,&VAR,vars);
                asprintf(&CodeSts,"%s\treturn %s\n",CoseExp,VAR); /*מציאת הקוד והחזרתו*/
            }
            else
            {
                asprintf(&CodeSts,"\treturn \n"); /*מציאת הקוד והחזרתו*/
            }
            line=line+1;
            return CodeSts;
            break;
        case IFELSE:
            CodeSts=GenIF(tree->left,size,vars);    /*מציאת הקוד והחזרתו*/
            return CodeSts;
            break;
        case LOOPS:
            CodeSts=GenLoop(tree->left->left,size,vars);  /*מצית הקוד והחזרתו*/
            return CodeSts;
            break;
        case FUNC:
            save=tree->middle;
            tree->middle=NULL;
            CodeSts=GenProgram(tree);   /*מציאת הקוד והחזרתו*/
            tree->middle=save;
            return CodeSts;  
            break;
        case BLOCKK:
            CodeSts=GenFunction(tree->left->left,size,vars);
            return CodeSts;
            break;
        case END:
            return "";
            break;

    }
    return NULL;

}
char* GenLoop(node *tree,int *size,node **vars)
{
    char *IntCode;
    char *expCode;
    char *VarExp="";
    char *incCode;
    char *proCode;
    char *LoopCode;
    char *BeginLabel;
    char *AfterLabel;
    if(strcmp(tree->token,"FOR")==0)    /*עבור לולאת פור*/
    {
        IntCode=GenSimple(tree->left->left,size,vars);     /*קוד עבור התחול*/
        BeginLabel=freshLabel();                     /*יצירת לייבלים*/
        expCode=GenExp(tree->left->middle,size,&VarExp,vars); /*קוד עבור ביטוי*/
        if(strcmp(tree->middle->token,"BLOCK")!=0)
        {
            proCode=GenFunction(tree->middle,size,vars);   /*קוד עבודה הבלוק או פקודה אחת*/
        }
        else
        {
            proCode=GenFunction(tree->middle->left,size,vars);
        }
        incCode=GenSimple(tree->left->right,size,vars);   /*קוד עבור העלאת משתנה*/
        line=line+2;
        AfterLabel=freshLabel();
        asprintf(&LoopCode,"%s%s:%s\tIfz %s Goto %s\n%s%s\tGoto %s\n%s:",IntCode,BeginLabel,expCode,VarExp,AfterLabel,proCode,incCode,BeginLabel,AfterLabel);
    }
    else if(strcmp(tree->token,"WHILE")==0)   /*עבור לולאת WHILE*/
        {
            BeginLabel=freshLabel();
            expCode=GenExp(tree->left,size,&VarExp,vars);
            if(strcmp(tree->middle->token,"BLOCK")!=0)
                {
                    proCode=GenFunction(tree->middle,size,vars);
                }
                else
                {
                    proCode=GenFunction(tree->middle->left,size,vars);
                }
            line=line+1;
            AfterLabel=freshLabel();
            asprintf(&LoopCode,"%s:%s\tIfz %s Goto %s\n%s\tGoto %s\n%s:",BeginLabel,expCode,VarExp,AfterLabel,proCode,BeginLabel,AfterLabel);
        }
        else
        {
  
            BeginLabel=freshLabel();
            proCode=GenFunction(tree->left->left,size,vars);   /*עבור לולאת DO-WHILE*/
            expCode=GenExp(tree->middle->left,size,&VarExp,vars);  
            line=line+1;
            asprintf(&LoopCode,"%s:%s%s\tIf %s Goto %s\n",BeginLabel,proCode,expCode,VarExp,BeginLabel);
            
        }
    return LoopCode;
}


char* GenIF(node *tree,int *size,node **vars)
{
   char* CodeExp;
   char* VarExp="";
   char* CodeProg;
   char* codeIf;
   char* falsLabel;
   char* trueLabel;
   if(tree->right==NULL) /*if without else*/
   {
        CodeExp=GenExp(tree->left,size,&VarExp,vars); /*בניית קוד עבור ביטוי*/
        if(strcmp(tree->middle->token,"BLOCK")!=0)
        {
            CodeProg=GenFunction(tree->middle,size,vars);
        }
        else
        {
            CodeProg=GenFunction(tree->middle->left,size,vars);
        }
        line=line+1;
        falsLabel=freshLabel();
        asprintf(&codeIf,"%s\tIfz %s Goto %s\n%s%s:",CodeExp,VarExp,falsLabel,CodeProg,falsLabel);
   } 
   else                                   /*if with else*/
   {
        CodeExp=GenExp(tree->left,size,&VarExp,vars); /*בניית קוד עבור ביטוי*/
        line=line+1;
        if(strcmp(tree->middle->token,"BLOCK")!=0)
        {
            CodeProg=GenFunction(tree->middle,size,vars);
        }
        else
        {
            CodeProg=GenFunction(tree->middle->left,size,vars);
        }
        line=line+1;

        falsLabel=freshLabel();
        if(strcmp(tree->right->left->token,"BLOCK")!=0)
        {
            CodeProg=GenFunction(tree->right->left,size,vars);
        }
        else
        {
            CodeProg=GenFunction(tree->right->left->left,size,vars);
        }
        trueLabel=freshLabel();
        asprintf(&codeIf,"%s\tIfz %s Goto %s\n%s\tGoto %s\n%s:",CodeExp,VarExp,falsLabel,CodeProg,trueLabel,falsLabel);
        asprintf(&codeIf,"%s%s%s:",codeIf,CodeProg,trueLabel);


   }
   return codeIf;
}

int checksingel(node *tree)/*בדיקה של השמה פשוטה*/
{
    if(strcmp(tree->token,"*")==0)
        return 0;
    if(strcmp(tree->token,"/")==0)
        return 0;
    if(strcmp(tree->token,"+")==0)
        return 0;    
    if(strcmp(tree->token,"-")==0)
        return 0;        
    if(strcmp(tree->token,"OR")==0)
        return 0;    
    if(strcmp(tree->token,"AND")==0)
        return 0;    
    if(strcmp(tree->token,"!=")==0)
        return 0;    
    if(strcmp(tree->token,">")==0)
        return 0;    
    if(strcmp(tree->token,">=")==0)
        return 0;    
    if(strcmp(tree->token,"<")==0)
        return 0;    
    if(strcmp(tree->token,"<=")==0)
        return 0;  
    if(strcmp(tree->token,"!")==0)
        return 0;
    if(strcmp(tree->token,"==")==0)
        return 0; 
    if(strcmp(tree->token,"FuncExpr")==0)
        return 0;  
    return 1;
}
char* GenSimple(node *tree,int *size,node **vars)  /*בניית קוד עבור פעולה פשוטה*/
{
    char *CodeExp;
    char *VarExp;
    char *VarHelp;
    char *VarHelp2;
    char *CodeSimple="";
    char *help;
    if(strcmp(tree->token,"=")==0)
    {
        if(checksingel(tree->middle)==1)    /*השמה יחידה*/
        {
            if(strcmp(tree->left->token,"pointer")==0)  /*השמה לתוך מצביע*/
            {
                
                CodeExp=GenExp(tree->middle,size,&VarExp,vars);/*קוד עבור צד ימין*/
                help=CodeExp;
                CodeExp=VarExp;
                VarExp=freshVar();
                asprintf(&help,"%s\t%s = %s\n",help, VarExp,CodeExp); /*שמירת הערך ברגיסטר*/
                line=line+1;
                VarHelp=VarExp;/*שמירת הרגיסטר*/
                CodeExp=GenExp(tree->left->left,size,&VarExp,vars);/*בניית הקוד עבור צד ימין */
                VarHelp2=freshVar();
                asprintf(&CodeSimple,"%s\t*(%s) = %s\n", help,VarHelp2,VarHelp);
                line=line+1;
                if(strcmp(tree->left->type,"CHARP")==0)
                    {
                        if(findVar(vars,VarHelp)==0)
                            {
                                *size=*size+getSize("CHAR");
                                *vars=AddVar(vars,VarHelp);
                                
                            }
                        if(findVar(vars,VarHelp2)==0)
                            {
                                *size=*size+getSize("CHARP");
                                *vars=AddVar(vars,VarHelp2);
                            }
                    }
                else{
                        if(findVar(vars,VarHelp)==0)
                            {
                                *size=*size+getSize("INT");
                                *vars=AddVar(vars,VarHelp);
                            }
                        if(findVar(vars,VarHelp2)==0)
                            {
                                *size=*size+getSize("INTP");
                                *vars=AddVar(vars,VarHelp2);
                            }   
                    }
            }

            else if(tree->left->left!=NULL && strcmp(tree->left->token,"pointer")!=0)  /*השמה לתוך מערך*/
            {
                CodeExp=GenExp(tree->middle,size,&VarExp,vars);/*קוד עבור צד ימין*/
                help=CodeExp;
                CodeExp=VarExp;
                VarExp=freshVar();
                asprintf(&help,"%s\t%s = %s\n",help, VarExp,CodeExp); /*שמירת הערך ברגיסטר*/
                line=line+1;
                if(findVar(vars,VarExp)==0)
                    {
                        *size=*size+getSize("CHAR");
                        *vars=AddVar(vars,VarExp);
                    }
                VarHelp=VarExp;/*שמירת הרגיסטר*/
                CodeExp=GenExp(tree->left->middle->left,size,&VarExp,vars);/*בניית הקוד עבור הביטוי המספרי*/
                VarHelp2=freshVar();
                if(findVar(vars,VarHelp2)==0)
                    {
                        *size=*size+getSize("CHARP");
                        *vars=AddVar(vars,VarHelp2);
                    }
                asprintf(&help,"%s%s\t%s = %s + %s\n",help,CodeExp, VarHelp2,tree->left->left->token,VarExp);/*חישבו מקום במערך*/
                line=line+1;
                asprintf(&CodeSimple,"%s\t*(%s) = %s\n", help,VarHelp2,VarHelp);
                line=line+1;
            }
            else
            {
                CodeExp=GenExp(tree->middle,size,&VarExp,vars);
                help=VarExp;
                VarExp=freshVar();
                asprintf(&CodeSimple,"%s\t%s = %s\n\t%s = %s\n",CodeExp, VarExp,help,tree->left->token,VarExp);
                line=line+2;
                if(findVar(vars,VarExp)==0)
                    {
                        *size=*size+getSize(tree->middle->type);
                        *vars=AddVar(vars,VarExp);
                    }
                if(findVar(vars,tree->left->token)==0)
                    {
                        *size=*size+getSize(tree->left->type);
                        *vars=AddVar(vars,tree->left->token);
                    }             
            }
        }
        else          /*עבור השמה מרובה*/
        {
            if(strcmp(tree->left->token,"pointer")==0)  /*השמה לתוך מצביע*/
            {
                CodeExp=GenExp(tree->middle,size,&VarExp,vars);
                asprintf(&CodeSimple,"%s\t*(%s) = %s\n", CodeExp,tree->left->left->token,VarExp);
                line=line+1;
                if(strcmp(tree->left->type,"CHARP")==0)
                    {
                        if(findVar(vars,tree->left->left->token)==0)
                            {
                                *size=*size+getSize("CHARP");
                                *vars=AddVar(vars,tree->left->left->token);
                            } 
                    }
                else{
                        if(findVar(vars,tree->left->left->token)==0)
                            {
                                *size=*size+getSize("INTP");
                                *vars=AddVar(vars,tree->left->left->token);
                            }
                    }
            }
            else            /*השמה לתוך משתנה רגיל*/
            {
                CodeExp=GenExp(tree->middle,size,&VarExp,vars);

                if(findVar(vars,tree->left->token)==0)
                    {
                        *size=*size+getSize(tree->left->type);
                        *vars=AddVar(vars,tree->left->token);
                    }
                asprintf(&CodeSimple,"%s\t%s = %s\n", CodeExp,tree->left->token,VarExp);
                line=line+1;
            }
        }
    }
    if(strcmp(tree->token,"callFunc")==0)   /*קריאה לפונקציה*/
    {
        CodeSimple=GenExp(tree,size,&VarExp,vars);

    }
    return CodeSimple;

}

char* GenExp(node *tree,int *size,char **VAR,node **vars) /*בניית קוד עבור ביטוי*/
{
    char *E1var="";
    char *E2var="";
    char *E1code;
    char *E2code;
    char *CodeExp="";
    char *helper1="";
    char *helper2="";
    int before;
    if(strcmp(tree->token,"!")==0)   /*ביוטי עם !*/
            {
                E1code=GenExp(tree->left,size,&E1var,vars);
                if(findVar(vars,*VAR)==0)
                    {
                        *size = *size + getSize("BOOLEAN");
                        *vars=AddVar(vars,*VAR);
                    }
                *VAR=freshVar();
                asprintf(&CodeExp,"%s\t%s = !%s\n", E1code,*VAR,E1var);
                line=line+1;
                return CodeExp;
            }
    if(strcmp(tree->token,"identifier")==0)  /*עבור משתנה*/
        {
            if(tree->left->left == NULL)
            {
                asprintf(VAR,"%s",tree->left->token);
                return "";
            }
            else
            {
                E1code=GenExp(tree->left->middle->left,size,&E1var,vars);/*בניית הקוד עבור הביטוי המספרי*/
                E2var=freshVar();
                if(findVar(vars,E2var)==0)
                    {
                        *size=*size+getSize("CHARP");
                        *vars=AddVar(vars,E2var);
                    }
                asprintf(&E1code,"%s\t%s = %s + %s\n",E1code,E2var,tree->left->left->token,E1var);/*חישבו מקום במערך*/
                line=line+1;
                *VAR=freshVar();
                if(findVar(vars,*VAR)==0)
                    {
                        *size=*size+getSize("CHARP");
                        *vars=AddVar(vars,*VAR);
                    }
                asprintf(&CodeExp,"%s\t%s = *(%s)\n",E1code,*VAR,E2var);/*חישבו מקום במערך*/
                line=line+1;
                return CodeExp;
            }
        }
    if(strcmp(tree->token,"length/absolute")==0)  /*עבור מספר*/
        {
            E1code=GenExp(tree->left,size,&E1var,vars);
            asprintf(VAR,"|%s|",E1var);
            return "";
        }

    if(strcmp(tree->token,"NUM")==0)  /*עבור מספר*/
        {
            asprintf(VAR,"%s",tree->left->token);
            return "";
        }
    if(strcmp(tree->token,"boolean")==0) /*עבור בולייאן*/
        {
            asprintf(VAR,"%s",tree->left->token);
            return "";
        }
    if(strcmp(tree->token,"CHAR")==0)  /*עבור תו*/
        {
            asprintf(VAR,"%s",tree->left->token);
            return "";
        }
    if(strcmp(tree->token,"string")==0)  /*עבור מחרוזת*/
        {
            asprintf(VAR,"%s",tree->left->token);
            return "";
        }
    
    if ((strcmp(tree->token, "*") == 0) || (strcmp(tree->token, "/") == 0)||((strcmp(tree->token, "+") == 0) || (strcmp(tree->token, "-") == 0)) && (tree->middle != NULL))  /*עבור פעולת מתמטיות*/
        {
            E1code=GenExp(tree->left,size,&E1var,vars);
            E2code=GenExp(tree->middle,size,&E2var,vars);
            *VAR=freshVar();
            if(findVar(vars,*VAR)==0)
                {
                    *size = *size + getSize("INT");
                    *vars=AddVar(vars,*VAR);
                }
            asprintf(&CodeExp,"%s%s\t%s = %s %s %s\n", E1code,E2code,*VAR,E1var,tree->token,E2var);
            line=line+1;
            return CodeExp;
        }
    if (((strcmp(tree->token, "+") == 0) || (strcmp(tree->token, "-") == 0)) && (tree->middle == NULL)) //plus & minus
        {
            E1code=GenExp(tree->left,size,&E1var,vars);
            *VAR=freshVar();
            asprintf(&CodeExp,"%s%s\t%s = %s %s \n", E1code,E2code,*VAR,tree->token,E1var);
            line=line+1;
            if(findVar(vars,*VAR)==0)
                {
                    *size = *size + getSize("INT");
                    *vars=AddVar(vars,*VAR);
                }
            return CodeExp;
        }
    
    if (strcmp(tree->token,"<=")==0 || strcmp(tree->token,"<")==0 || strcmp(tree->token,">=")==0 || strcmp(tree->token,">")==0 || strcmp(tree->token, "OR") == 0 || strcmp(tree->token, "AND") == 0|| strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0)  /*עבור פעולת החזירות בוליאן*/
        {
            E1code=GenExp(tree->left,size,&E1var,vars);
            if(tree->middle->middle==NULL)
            {  
                E2var=freshVar();
                E2code=GenExp(tree->middle,size,&helper1,vars);
                asprintf(&E2code,"\t%s = %s\n",E2var,helper1);
                line=line+1;
                if(findVar(vars,E2var)==0)
                    {
                        *size = *size + getSize(tree->middle->type);
                        *vars=AddVar(vars,E2var);
                    }
            }
            else
                E2code=GenExp(tree->middle,size,&E2var,vars);
            *VAR=freshVar();
            if(strcmp(tree->token, "OR") == 0)
                asprintf(&CodeExp,"%s%s\t%s = %s %s %s\n", E1code,E2code,*VAR,E1var,"||",E2var);
            else if(strcmp(tree->token, "AND") == 0)             
                asprintf(&CodeExp,"%s%s\t%s = %s %s %s\n", E1code,E2code,*VAR,E1var,"&&",E2var);
            else asprintf(&CodeExp,"%s%s\t%s = %s %s %s\n", E1code,E2code,*VAR,E1var,tree->token,E2var);
            line=line+1;
            if(findVar(vars,*VAR)==0)
                {
                    *size = *size + getSize("BOOLEAN");
                    *vars=AddVar(vars,*VAR);
                }
            return CodeExp;
        }
        
    if (strcmp(tree->token,"FuncExpr")==0)  /*עבור קריאה לפונקציה*/
    {
        before=*size;
        E1code=GenParm(tree->middle,size,vars);
        before=*size-before;
        *VAR=freshVar();
        if(before!=0)
        {
            asprintf(&CodeExp,"%s\t%s = %s %s\n\t%s %d\n", E1code,*VAR,"LCall",tree->left->token,"PopParm",before);
            line=line+2;
        }
        else
        {
            asprintf(&CodeExp,"%s\t%s = %s %s\n", E1code,*VAR,"LCall",tree->left->token);
            line=line+2;
        }
        if(findVar(vars,*VAR)==0)
                {
                    *size = *size + getSize(tree->left->type);
                    *vars=AddVar(vars,*VAR);
                }
        return CodeExp;
    }
    if (strcmp(tree->token,"callFunc")==0)  /*עבור קריאה לפונקציה*/
    {
        before=*size;
        E1code=GenParm(tree->middle,size,vars);
        before=*size-before;
        *VAR=freshVar();
        if(before!=0)
        {
            asprintf(&CodeExp,"%s\t%s %s\n\t%s %d\n", E1code,"LCall",tree->left->token,"PopParm",before);
            line=line+2;
        }
        else
        {
            asprintf(&CodeExp,"%s\t%s %s\n", E1code,"LCall",tree->left->token);
            line=line+1;
        }
        return CodeExp;
    }
    if (strcmp(tree->token,"pointer")==0)  /*עבור קריאה לפוינטר*/
    {

        CodeExp=GenExp(tree->left,size,&E2var,vars);
        asprintf(VAR,"*(%s)",E2var);
        return CodeExp;

    }
    if (strcmp(tree->token,"addres")==0)  /*עבור קריאה לפוינטר*/
    {
        CodeExp=GenExp(tree->left,size,&E2var,vars);
        asprintf(VAR,"&(%s)",E2var);
        return CodeExp;
    }

    
}
char *GenParm(node *tree,int *size,node **vars)/*החזרת קוד עבור פרמטרים*/
{
    char *ParmCode="",*var="";
    char *temp;
    node *sts=tree;
    while(sts!=NULL)
    {
        temp=GenExp(sts->left,size,&var,vars);
        asprintf(&ParmCode,"%s\tPushParm %s\n", ParmCode, var);
        line=line+1;
        if(findVar(vars,var)==0)
            {
                *size=*size+getSize(sts->left->type);
                *vars=AddVar(vars,var);
            }
        sts=sts->middle;
    }
    return ParmCode;
}
