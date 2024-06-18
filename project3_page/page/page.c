#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//used to transfer datatype
union Number
{
    unsigned int i;
    char c[4];
};

//structure for storing information
struct PAGEINFO
{
    unsigned int index;
    unsigned int offset;
};
typedef struct PAGEINFO PI;

const int len = 32;
//const int n = 16;


//process the address & mask them into two part
PI Process(char* A, union Number *num, int n, unsigned int* NumMask, unsigned int* OffsetMask)
{
    sprintf(A, "%s", A+2);
    char a, b;
    for(int i=7,j=0; i>=0; i-=2,j++){
        a = (A[i]>90)?A[i]-'W':A[i]-'0';
        b = (A[i-1]>90)?A[i-1]-'W':A[i-1]-'0';
        num->c[j] = a|(b<<4);
    }
    
    PI result;
    result.index = ((*NumMask)&(num->i))>>n;
    result.offset = (*OffsetMask)&(num->i);
    return result;
}


int main(int argc, char* argv[])
{
    //get parameters
    int n; 
    int checkformat = 0;
    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"-n") == 0 && i+1<argc){
            sscanf(argv[i+1], "%d", &n);
            checkformat = 1;
        }
    }
    if(!checkformat){
        printf("No size is assigned, n's default value is 16\n");
        n = 16;
    }
    
    //produce the mask
    unsigned int NumMask = 1;
    unsigned int OffsetMask = 1;
    for(int i=1;i<n;i++){
        OffsetMask = 1|(OffsetMask<<1);
    }
    NumMask = ~OffsetMask;

    //read file & store them into array
    char input [10000][25];
    int ip = 0;
    int c;    
    char temp [30];    

    FILE *fp;
    if((fp = fopen("test.txt", "r")) == NULL){
        printf("Can't open test.txt!\n");
        exit(EXIT_FAILURE);
    }
    
    union Number num;
    int tp = 0;
    while(fscanf(fp,"%s", temp) == 1){
        sprintf(input[ip++], "%s", temp);
    }
        
    fclose(fp);
    
    //start processing & print it out to group23_ans.txt
    if((fp = fopen("group23_ans.txt","w")) == NULL){
        printf("Can't open group23_ans.txt!\n");
        exit(EXIT_FAILURE);
    }

    PI ans;
    for(int i=0;i<ip;i++){
        ans = Process(input[i], &num, n, &NumMask, &OffsetMask);
        fprintf(fp, "%-10u %-10u\n", ans.index, ans.offset);
    }
    
    fclose(fp);
}
