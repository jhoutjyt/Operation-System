#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"
#include <string.h>

//define imformation structure
struct CacheInfo{
    int s, E, b;
    int setNum, blocknum;
};

struct Input{
    char operation;
    unsigned long long int address;
    int size; 
};

//define the cache structure
struct Block{
    
    unsigned long long int tag;
    struct Block* next;
};

struct Set{
    
    struct Block* first;
    int numbers;

};

struct Cache{
    struct Set* Sets;
};

//Cache Action
void GetBytes(struct Cache* CP, int s, int E, int b, struct Input I, int *Hit, int *Miss, int *Evict){
   
    //process the address into set id & tag
    unsigned long long int tag = I.address >> (s+b);
    unsigned long long int setid = (I.address << (64-s-b)) >> (64-s);
     
    if(I.operation == 'I')
        return;
    
    
    struct Block* iter = CP->Sets[setid].first;
    struct Block* pre;
    struct Block* temp;
    while(iter!=NULL){
        if(iter->tag == tag){
            if(I.operation == 'M')
                *(Hit) += 2;
            else
                (*Hit)++;
            //only one line in the set so no need to do LRU
            if(CP->Sets[setid].numbers == 1){
                return;
            }
            //put the current one to the bottom of the list
            else if(iter == CP->Sets[setid].first){
                CP->Sets[setid].first = iter->next;
            }
            else{
                pre->next = iter->next;
            }
            
            iter->next = NULL;
            temp = CP->Sets[setid].first;
            while(temp->next != NULL)
                temp = temp->next;
            temp->next = iter;
            return;
        }
        //keep finding
        else{
            if(iter->next != NULL){
                pre = iter;
                iter = iter->next;
            }
            else{
                break;
            }
        }
    
    }
    
    //if it's a miss, the program go down to this part
    (*Miss)++;
    

    //If the set is full but can't find the tag, delete the first one in the list(LRU)
    if(CP->Sets[setid].numbers == E){
        (*Evict)++;
        
        iter = CP->Sets[setid].first;
        CP->Sets[setid].first = CP->Sets[setid].first->next;
        free(iter);
        CP->Sets[setid].numbers --;
    }

    //get a new tag from memory 
    iter = (struct Block*)malloc(sizeof(struct Block));
    iter->next = NULL;
    iter->tag = tag;
    if(CP->Sets[setid].first == NULL)
        CP->Sets[setid].first = iter;
    else{
        pre = CP->Sets[setid].first;
        while(pre->next != NULL)
            pre = pre->next;
        pre->next = iter;   
    }    
    CP->Sets[setid].numbers ++;

    if(I.operation == 'M')
        (*Hit) ++;

    return;
}


//global values
int Hit = 0;
int Miss = 0;
int Evict = 0;
char* file;

int main(int argc, char** argv){
    
    //Read parameter   
    struct CacheInfo CI;    
    for(int i=0;i<argc;i++){
        
        if(strcmp(argv[i], "-s") == 0){        
            sscanf(argv[i+1], "%d", &CI.s);
        }
        else if(strcmp(argv[i], "-E") == 0){        
            sscanf(argv[i+1], "%d", &CI.E);
        }
        else if(strcmp(argv[i], "-b") == 0){        
            sscanf(argv[i+1], "%d", &CI.b);
        }
        else if(strcmp(argv[i], "-t") == 0){
           file = argv[i+1]; 
        }
    }
    
    printf("%d,%d,%d,%s\n",CI.s,CI.E,CI.b,file);
    

    
    //initialize the cache
    int setnums = 1 << CI.s;
    struct Cache C;
    C.Sets = malloc(sizeof(struct Set) * setnums);
    //memset(C.Sets, 0, sizeof(struct Set) * setnums); 

    //ReadFile
    FILE *fp;
    if((fp = fopen(file, "r")) == NULL){
        printf("Can't open file!\n");
        exit(EXIT_FAILURE);
    }
    

    struct Input DATA;
    
    char OP;
    unsigned long long int ad;
    int size;
    int read = 0;
    
    while(1){
        
        read = fscanf(fp, "%c %llx,%d",&OP,&ad,&size);
        if(read == EOF)
            break;
        else if(read == 3){
        
            DATA.operation = OP;
            DATA.address = ad;
            DATA.size = size;
            GetBytes(&C, CI.s, CI.E, CI.b, DATA, &Hit, &Miss, &Evict);
        }
        
       
    }

    printSummary(Hit,Miss,Evict);
    
    
    return 0;
}


