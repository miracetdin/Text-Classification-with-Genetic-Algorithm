#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// define the data structure (text - class)
typedef struct data DATA;
struct data{
    char *class;
    char *text;
};

// define a cache structure
typedef struct cache CACHE;
struct cache{
    DATA *dataSet;
    int num;
    int length;
};

// define a dictionary structure
typedef struct dictionary DICTIONARY;
struct dictionary{
    char *word;
    char *class;
    int frequency;
};

// define a cromosom structure 

// function prototypes
FILE *open_file(char *fileName);
CACHE *read_file(char *fileName);
void print_dataSet(CACHE *cache);
DICTIONARY *create_dictionary(CACHE *cache);

int main(void){
    CACHE *cache;
    DICTIONARY *dict;

    char *fileName;
    int i;

    // reading the file
    cache = read_file("amazon_reviews.csv");

    // print the data set
    //print_dataSet(cache);

    dict = create_dictionary(cache);

    return 0;
}

FILE *open_file(char *fileName){
    FILE *f;

    if((f = fopen(fileName, "r")) == NULL){
        printf("ERROR 1: Cannot open the file!");
        exit(1);
    }

    return f;
}

CACHE *read_file(char *fileName){
    CACHE *cache;
    DATA data;
    FILE *file;
    DATA *dataSet;

    char c;
    int lines = 0;
    int counter = 0;
    int length = 0;
    char *line;
    int i;
    char *token;

    file = open_file(fileName);

    //count the lines (number of data) and find the max length of line
    for(c=getc(file); c!=EOF; c=getc(file)){
        if(c == '\n'){
            lines++;
            counter = 0;
        }

        counter++;
        if(counter > length){
            length = counter;
        }
    }
    lines++;
    length++;

    line = (char*)malloc(length*sizeof(char));
    if(line == NULL){
        printf("ERROR 2: line cannot be created!");
        exit(1);
    }

    dataSet = (DATA*)malloc(lines*sizeof(DATA));
    if(dataSet == NULL){
        printf("ERROR 3: dataSet cannot be created!");
        exit(1);
    }

    for(i=0; i<lines; i++){
        dataSet[i].class = (char*)malloc(sizeof(char));
        dataSet[i].text = (char*)malloc(length*sizeof(char));
    }

    // read the data
    fseek(file, 0, SEEK_SET);
    i = 0;
    while(!feof(file)){
        fgets(line, length, file);
        
        //read the class
        token = strtok(line, ",");
        strcpy(dataSet[i].class, token);

        // read the text
        token = strtok(NULL, "\n");
        strcpy(dataSet[i].text, token);

        i++;
    }

    cache = (CACHE*)malloc(sizeof(CACHE));
    if(cache == NULL){
        printf("ERROR 4: cache cannot be created!");
        exit(1);
    }

    cache->dataSet = dataSet;
    cache->num = lines;
    cache->length = length;

    free(line);
    free(dataSet);
    fclose(file);

    return cache;
}   

void print_dataSet(CACHE *cache){
    int i = 0;
    
    printf("Your Data Set:\n");
    printf("Number of data (lines): %d\n", cache->num);
    printf("Max length of lines: %d\n", cache->length);
    printf("Index\tClass\tText\n");
    printf("-----\t-----\t----\n");
    for(i=0; i<cache->num; i++){
        printf("%d", i+1);
        printf("\t");
        printf(cache->dataSet[i].class);
        printf("\t");
        printf(cache->dataSet[i].text);
        printf("\n");
    }
}

DICTIONARY *create_dictionary(CACHE *cache){
    DICTIONARY *dictionary1, *dictionary2;

    int i, j;
    int same = 0;
    char *token;
    int counter1, counter2 = 0;
    
    dictionary1 = (DICTIONARY*)malloc(1000*sizeof(DICTIONARY));
    if(dictionary1 == NULL){
        printf("ERROR 5: dictionary 1 cannot be created!");
        exit(1);
    }
    dictionary2 = (DICTIONARY*)malloc(1000*sizeof(DICTIONARY));
    if(dictionary2 == NULL){
        printf("ERROR 6: dictionary 2 cannot be created!");
        exit(1);
    }
    
    // assign the words to dictionaries
    counter1 = 0;
    counter2 = 0;
    printf("num: %d\n", cache->num);
    for(i=0; i<cache->num; i++){
        // disctionary 1 operations    
        if(strcmp(cache->dataSet[i].class, "1") == 0){
            token = strtok(cache->dataSet[i].text, " ");

            while(token){
                if(counter1 != 0){
                    for(j=0; j<counter1; j++){
                        if(strcmp(dictionary1[j].word, token) == 0){
                            dictionary1[j].frequency++;
                            same = 1;
                        } 
                    }
                    if(same == 0){
                        counter1++;
                        // TODO: realloc return  null
                        dictionary1 = (DICTIONARY*)realloc(dictionary1, (counter1+1)*sizeof(DICTIONARY));
                        dictionary1[counter1-1].word = token;
                        dictionary1[counter1-1].class = cache->dataSet[i].class;
                        dictionary1[counter1-1].frequency = 1;
                    }
                    else{
                        same = 0;
                    }
                }
                else{
                    counter1++;
                    // TODO: realloc return  null
                    dictionary1 = (DICTIONARY*)realloc(dictionary1, (counter1+1)*sizeof(DICTIONARY));
                    dictionary1[0].word = token;
                    dictionary1[0].class = cache->dataSet[i].class;
                    dictionary1[0].frequency = 1;
                }
                token = strtok(NULL, " ");
            }
        }
        // dictionary 2 operations
        else{
            token = strtok(cache->dataSet[i].text, " ");

            while(token){
                if(counter2 != 0){
                    for(j=0; j<counter2; j++){
                        if(strcmp(dictionary2[j].word, token) == 0){
                            dictionary2[j].frequency++;
                            same = 1;
                        } 
                    }
                    if(same == 0){
                        counter2++;
                        // TODO: realloc return  null
                        dictionary2 = (DICTIONARY*)realloc(dictionary2, (counter2+1)*sizeof(DICTIONARY));
                        dictionary2[counter2-1].word = token;
                        dictionary2[counter2-1].class = cache->dataSet[i].class;
                        dictionary2[counter2-1].frequency = 1;
                    }
                    else{
                        same = 0;
                    }
                }
                else{
                    counter2++;
                    // TODO: realloc return  null
                    dictionary2 = (DICTIONARY*)realloc(dictionary2, (counter2+1)*sizeof(DICTIONARY));
                    dictionary2[0].word = token;
                    dictionary2[0].class = cache->dataSet[i].class;
                    dictionary2[0].frequency = 1;
                }
                token = strtok(NULL, " ");
            }
        }
    }
    
    printf("Dictionary 1 size: %d\n", counter1);
    for(i=0; i<5; i++){
        printf(dictionary1[i].word);
        printf("\t");
        printf(dictionary1[i].class);
        printf("\t");
        printf("%d\n", dictionary1[i].frequency);
    }

    printf(dictionary1[0].word);
    printf("\n");
    printf(dictionary1[1867].word);

    printf("\nDictionary 2 size: %d\n", counter2);
    for(i=0; i<5; i++){
        printf(dictionary2[i].word);
        printf("\t");
        printf(dictionary2[i].class);
        printf("\t");
        printf("%d\n", dictionary2[i].frequency);
    }

    printf(dictionary2[0].word);
    printf("\n");
    printf(dictionary2[1802].word);

    return dictionary1;
}