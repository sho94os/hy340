#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define HASH_SIZE 50

/* typoi mias eggrafis toy symbol table */
typedef enum SymbolTableType{ GLOBAL, LOCALVAR, FORMAL, USERFUNC, LIBFUNC }Type;

typedef struct SymbolTableEntry{
    int isActive;
    unsigned int scope;
    unsigned int line;
    char * name;
    enum SymbolTableType type;
    struct SymbolTableEntry* next_scope;
    struct SymbolTableEntry* next_link;
} symrec_t;

/* Head of the scope list */
symrec_t* scope_Head = NULL;

/* hash table me ta record*/
typedef struct HashTable{
    int hash_count;
    symrec_t *hashTable[HASH_SIZE];
} table_t;

table_t* myHashTable;

/* hash function*/
int hashCode(int key) {
   return  key % HASH_SIZE;
}

table_t * newHashTable(){
   int i;
   myHashTable = malloc(sizeof(table_t));
   myHashTable->hash_count=0;

   for(i=0; i<HASH_SIZE; i++){
      myHashTable->hashTable[i] = NULL;
   }

   return myHashTable;
}

symrec_t * NewScopeNode(int scope,int isActive){
    /*create a "head" node to scopes list*/
    symrec_t * temp = malloc(sizeof(symrec_t));
    temp->isActive = isActive; /* -1 is the Active number of the list*/
    temp->next_link = NULL;
    temp->next_scope = NULL;
    temp->scope = scope;

    return temp;
}

/*pairnei ws parametro twn recordTable kai to neo record pou theloume na eisagoume
 *kai me bash thn hashFunction thn kanei eisagwgh*/
void insert(int scope, int line, Type type, char* name){
    int key = hashCode(myHashTable->hash_count++);
    symrec_t * temp = malloc(sizeof(symrec_t));
    symrec_t * scope_temp;
    symrec_t * scope_temp2;
    symrec_t * scope_temp3;
    symrec_t * prev = NULL;
    temp->isActive = 1;
    temp->type = type;
    temp->scope = scope;
    temp->line = line;
    temp->name = name;
    temp->next_link = NULL;
    temp->next_scope = NULL;

    /*insert temp to hashtable first*/
    if(myHashTable->hashTable[key] == NULL){
        myHashTable->hashTable[key] = temp;
    }else{
        /*insert first to list*/
        temp->next_link = myHashTable->hashTable[key];
        myHashTable->hashTable[key] = temp;
    }

    /*now link the hashtable with scopes list*/
    if(scope_Head == NULL){
        scope_Head = NewScopeNode(scope, -1);
        scope_temp = scope_Head;
    }else{
        scope_temp = scope_Head;
        while(scope_temp->next_link != NULL && scope_temp->scope < scope ){ 
            prev = scope_temp;
            scope_temp=scope_temp->next_link;
        }
        if(scope_temp->next_link == NULL && scope_temp->scope < scope){
            scope_temp2 = NewScopeNode(scope, -1);
            scope_temp->next_link = scope_temp2;
            scope_temp2->next_link = NULL;
            scope_temp = scope_temp2;
        }else if(scope_temp->scope > scope){
            scope_temp2 = NewScopeNode(scope, -1);
            scope_temp2->next_link = prev->next_link;
            prev->next_link = scope_temp2;
            scope_temp = scope_temp2;
        }
    }

    /*insert sto telos tis scope(kathetis) list*/
    scope_temp3 = scope_temp;
    while(scope_temp3->next_scope != NULL){
        scope_temp3 = scope_temp3->next_scope;
    }
    myHashTable->hashTable[key]->next_scope = scope_temp3->next_scope;
    scope_temp3->next_scope = myHashTable->hashTable[key];
}
symrec_t * find_scope_head(int scope){
    
    symrec_t * temp = scope_Head;
    while(temp->next_link != NULL && temp->scope != scope){
        temp = temp->next_link;
    }
    if(temp->next_link == NULL){
        if(temp->scope == scope){
            return temp;
        }
        return NULL;
    }
    return temp;
}

void hide_scope(int scope){
    symrec_t * temp;
    symrec_t * scope_list_head;
    temp = scope_Head;
    
    if(scope == 0){
        fprintf(stderr, "ERROR, can't hide GLOBAL scope (0)!\n");
        return;
    }
    temp = find_scope_head(scope);
    if(temp == NULL){
        return;
    }
    scope_list_head = temp->next_scope;
    while(scope_list_head != NULL){
        scope_list_head->isActive = 0; /* hide record */
        scope_list_head = scope_list_head->next_scope;
    }
}

int isHidden(symrec_t * record){
    if(record->isActive == 0){
        return 1; /**/
    }
    return 0;
}

symrec_t * lookup(char * sym_name, int scope){
    symrec_t * temp;
    symrec_t * scope_list_head;
    temp = scope_Head;
    
    temp = find_scope_head(scope);
    
    
    if(temp == NULL){
        return NULL; /*scope not found*/
    }
    scope_list_head = temp->next_scope;
    while(scope_list_head != NULL && (strcmp(scope_list_head->name, sym_name) || isHidden(scope_list_head)) ){
        scope_list_head = scope_list_head->next_scope;
    }
    
    if(scope_list_head != NULL){
        if(isHidden(scope_list_head)){
            return NULL;
        }
        return scope_list_head; /*found*/
    }else{
        return NULL; /*SYMBOL not found*/
    }
}

symrec_t * lookupFormal(char * sym_name, int scope){
    /*idia me lookup apla prosthetoume mia epipleon 
    && scope_list_head->type == FORMAL*/
    symrec_t * temp;
    symrec_t * scope_list_head;
    temp = scope_Head;
    temp = find_scope_head(scope);
   
    
    if(temp == NULL){
        return NULL; /*scope not found*/
    }
    scope_list_head = temp->next_scope;
    while(scope_list_head != NULL && (strcmp(scope_list_head->name, sym_name) || scope_list_head->type != FORMAL)){
        scope_list_head = scope_list_head->next_scope;
    }
    if(scope_list_head != NULL){
        if(isHidden(scope_list_head)){
            return NULL;
        }
        return scope_list_head; /*found*/
    }else{
        return NULL; /*SYMBOL not found*/
    }
}

symrec_t * lookupAbove(char * sym_name, int scope){
    /*look up above scopes except global scope*/
    symrec_t * temp = scope_Head;
    symrec_t * temp2 = scope_Head;
    while(scope > 1){
        /*temp = find_scope_head(--scope);*/
        scope--;
        temp2 = lookup(sym_name, scope);

        if(temp2 != NULL){
            return temp2;
        }
    }
    return NULL;
}

symrec_t * lookupLIBFUNCTs(char * name){
    /*pairneis tin katheti tou  scope 0 pou stin arxi tis einai einai oi libfuncts*/
    symrec_t * katheti = scope_Head->next_scope; 
    while(katheti->next_scope != NULL && katheti->type == LIBFUNC && strcmp(katheti->name, name)){
        katheti = katheti->next_scope;
    }
    if(!strcmp(katheti->name, name) && katheti->type == LIBFUNC){
        /*to vrika*/
        return katheti;
    }
    return NULL;
}

void insert_LIBFUNCTs(){
    insert(0, 0, LIBFUNC, "print");
    insert(0, 0, LIBFUNC, "input");
    insert(0, 0, LIBFUNC, "objectmemberkeys");
    insert(0, 0, LIBFUNC, "objecttotalmembers");
    insert(0, 0, LIBFUNC, "objectcopy");
    insert(0, 0, LIBFUNC, "totalarguments");
    insert(0, 0, LIBFUNC, "argument");
    insert(0, 0, LIBFUNC, "typeof");
    insert(0, 0, LIBFUNC, "strtonum");
    insert(0, 0, LIBFUNC, "sqrt");
    insert(0, 0, LIBFUNC, "cos");
    insert(0, 0, LIBFUNC, "sin");
}

void print_results(){
    int i;
    symrec_t* orizontia;
    symrec_t* katheta;
    orizontia = scope_Head;
    katheta = scope_Head;
    while(orizontia != NULL){
        printf("\n----------    Scope: #%d    ----------\n", orizontia->scope);
        katheta = orizontia;
        while(katheta->next_scope != NULL){
            switch(katheta->next_scope->type){
                case FORMAL:
                    printf("\"%s\" [formal argument] (line: %d) (scope: %d)\n",katheta->next_scope->name, katheta->next_scope->line, katheta->next_scope->scope);
                    break;
                case USERFUNC:
                    printf("\"%s\" [user function] (line: %d) (scope: %d)\n",katheta->next_scope->name, katheta->next_scope->line, katheta->next_scope->scope);
                    break;
                case GLOBAL:
                    printf("\"%s\" [global variable] (line: %d) (scope: %d)\n",katheta->next_scope->name, katheta->next_scope->line, katheta->next_scope->scope);
                    break;
                case LOCALVAR:
                    printf("\"%s\" [local variable] (line: %d) (scope: %d)\n",katheta->next_scope->name, katheta->next_scope->line, katheta->next_scope->scope);
                    break;
                case LIBFUNC:
                    printf("\"%s\" [library function] (line: %d) (scope: %d)\n",katheta->next_scope->name, katheta->next_scope->line, katheta->next_scope->scope);
                    break;
                default:
                    break;
            }
            katheta->next_scope = katheta->next_scope->next_scope;
        }
        orizontia = orizontia->next_link;
    }
    return;
}