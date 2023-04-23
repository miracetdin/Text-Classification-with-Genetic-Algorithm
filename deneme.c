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

// define a dictionary cache
typedef struct dict_cache DICT_CACHE;
struct dict_cache{
    DICTIONARY *dict1;
    int num1;
    DICTIONARY *dict2;
    int num2;
};

typedef struct nucleotide NUCLEOTIDE;
struct nucleotide{
    char *word;
    int frequency;
};

// define a indivual structure
typedef struct individual INDIVIDUAL;
struct individual{
    NUCLEOTIDE *class1;
    NUCLEOTIDE *class2;
    int num;
    float fitness;
};

// function prototypes
FILE *open_file(char *fileName);
CACHE *read_file(char *fileName);
void print_dataSet(CACHE *cache);
DICT_CACHE *create_dictionary(CACHE *cache);
void print_dictionaries(DICT_CACHE *dict_cache);
DICT_CACHE *filter_dicts(DICT_CACHE *dict_cache, float thresholdSame, float thresholdExtreme);
int compare_frequencies(const void *a, const void *b);
void genetic_algorithm(DICT_CACHE * dict_cache, int numWord, int numIndiv, float mutatRate);

int main(void){
    CACHE *cache;
    DICT_CACHE *dict_cache;

    int numWord, numIndiv;
    float thresholdSame=0.33, thresholdExtreme=0.10, mutatRate;

    // reading the file
    cache = read_file("amazon_reviews.csv");

    // print the data set
    printf("\n\n.....DATA SET.....\n\n");
    print_dataSet(cache);
    
    // creating dictionaries
    dict_cache = create_dictionary(cache);

    // print the dictionaries
    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n\n.....DICTIONARIES.....\n\n");
    print_dictionaries(dict_cache);

    // make dictionaries unique
    dict_cache = filter_dicts(dict_cache, thresholdSame, thresholdExtreme);

    // print the dictionaries
    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n\n.....FLTERED DICTIONARIES.....\n\n");
    print_dictionaries(dict_cache);

    // print the dictionaries
    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n\n.....GENETIC ALGORITHM.....\n\n");
    //genetic_algorithm(dict_cache, numWord, numIndiv, mutatRate);

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
    
    printf("Data Set Info:\n");
    printf("Number of data (lines): %d\n", cache->num);
    printf("Max length of lines: %d\n\n", cache->length);
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

DICT_CACHE *create_dictionary(CACHE *cache){
    DICT_CACHE *dict_cache;
    DICTIONARY *dictionary1, *dictionary2;

    char *class1;
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
    
    class1 = (char*)malloc(sizeof(char));
    if(class1 == NULL){
        printf("ERROR 7: class 1 cannot be created!");
        exit(1);
    }

    // assign the classes from data set
    class1 = cache->dataSet[0].class;

    // assign the words to dictionaries
    counter1 = 0;
    counter2 = 0;
    for(i=0; i<cache->num; i++){
        // dictionary 1 operations    
        if(strcmp(cache->dataSet[i].class, class1) == 0){
            // get the first word of the text
            token = strtok(cache->dataSet[i].text, " ");

            while(token){
                // if the dictionary is not empty
                if(counter1 != 0){
                    for(j=0; j<counter1; j++){
                        // if the word is already in the dictionary
                        if(strcmp(dictionary1[j].word, token) == 0){
                            dictionary1[j].frequency++;
                            same = 1;
                        } 
                    }
                    // if the word is not in the dictionary
                    if(same == 0){
                        counter1++;
                        // TODO: realloc return  null
                        dictionary1 = (DICTIONARY*)realloc(dictionary1, (counter1+1)*sizeof(DICTIONARY));
                        dictionary1[counter1].word = token;
                        dictionary1[counter1].class = cache->dataSet[i].class;
                        dictionary1[counter1].frequency = 1;
                    }
                    else{
                        same = 0;
                    }
                }
                // if the dictionary is not empty
                else{
                    counter1++;
                    // TODO: realloc return  null
                    dictionary1 = (DICTIONARY*)realloc(dictionary1, (counter1+1)*sizeof(DICTIONARY));
                    dictionary1[counter1].word = token;
                    dictionary1[counter1].class = cache->dataSet[i].class;
                    dictionary1[counter1].frequency = 1;
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
                        dictionary2[counter2].word = token;
                        dictionary2[counter2].class = cache->dataSet[i].class;
                        dictionary2[counter2].frequency = 1;
                    }
                    else{
                        same = 0;
                    }
                }
                else{
                    counter2++;
                    // TODO: realloc return  null
                    dictionary2 = (DICTIONARY*)realloc(dictionary2, (counter2+1)*sizeof(DICTIONARY));
                    dictionary2[counter2].word = token;
                    dictionary2[counter2].class = cache->dataSet[i].class;
                    dictionary2[counter2].frequency = 1;
                }
                token = strtok(NULL, " ");
            }
        }
    }

    dict_cache = (DICT_CACHE*)malloc(sizeof(DICT_CACHE));
    if(dict_cache == NULL){
        printf("ERROR 8: dict_cache cannot be created!");
        exit(1);
    }
    
    dict_cache->dict1 = dictionary1;
    dict_cache->num1 = counter1;
    dict_cache->dict2 = dictionary2;
    dict_cache->num2 = counter2;

    return dict_cache;
}

void print_dictionaries(DICT_CACHE *dict_cache){
    int i;

    printf("Dictionary 1 size: %d\n", dict_cache->num1);
    printf("Word\tClass\tFrequency\n");
    printf("----\t-----\t---------\n");
    for(i=1; i<=5; i++){
        printf(dict_cache->dict1[i].word);
        printf("\t");
        printf(dict_cache->dict1[i].class);
        printf("\t");
        printf("%d\n", dict_cache->dict1[i].frequency);
    }
   
    printf("\nDictionary 2 size: %d\n", dict_cache->num2);
    printf("Word\tClass\tFrequency\n");
    printf("----\t-----\t---------\n");
    for(i=1; i<=5; i++){
        printf(dict_cache->dict2[i].word);
        printf("\t");
        printf(dict_cache->dict2[i].class);
        printf("\t");
        printf("%d\n", dict_cache->dict2[i].frequency);
    }    
}

DICT_CACHE *filter_dicts(DICT_CACHE *dict_cache, float thresholdSame, float thresholdExtreme){
    DICTIONARY *dictionary1, *dictionary2;

    int i, j;
    int counter1, counter2;
    float freqRate, freqRate1, freqRate2;

    // define two pointers
    dictionary1 = (DICTIONARY*)malloc(sizeof(DICTIONARY));
    if(dictionary1 == NULL){
        printf("ERROR 9: dictionary1 cannot be created!");
        exit(1);
    }

    dictionary2 = (DICTIONARY*)malloc(sizeof(DICTIONARY));
    if(dictionary2 == NULL){
        printf("ERROR 10: dictionary2 cannot be created!");
        exit(1);
    }

    dictionary1 = dict_cache->dict1;
    dictionary2 = dict_cache->dict2;
    counter1 = dict_cache->num1;
    counter2 = dict_cache->num2;

    // if these dictionaries have the same word
    for(i=1; i<dict_cache->num1; i++){
        for(j=1; j<dict_cache->num2; j++){
            // dictionaries have the same word
            if(strcmp(dictionary1[i].word, dictionary2[j].word) == 0){
                // calculate the frequencies of the word for each class
                freqRate1 = (float)(dictionary1[i].frequency) / dict_cache->num1;
                freqRate2 = (float)(dictionary2[j].frequency) / dict_cache->num2;
                // the frequency of the class 1 is smaller
                if(freqRate1 < freqRate2){
                    // how many times smaller
                    freqRate = (float)(freqRate1) / freqRate2;
                    // if it smaller than threshold, delete it
                    if(thresholdSame > freqRate){
                        dictionary1[i].word = "";
                        counter1--;
                    }
                }
                // the frequency of the class 2 is smaller
                else if(freqRate2 < freqRate1){
                    freqRate = (float)(freqRate2) / freqRate1;
                    if(thresholdSame > freqRate){
                        dictionary2[j].word = "";
                        counter2--;
                    }
                }
                // the frequencies are equal, delete them
                else{
                    dictionary1[i].word = "";
                    counter1--;
                    dictionary2[j].word = "";
                    counter2--;
                }
                j = dict_cache->num2;
            }
        }
    }

    // dictionaries are re-created with new sizes
    dict_cache->dict1 = (DICTIONARY*)realloc(dict_cache->dict1, counter1*sizeof(DICTIONARY));
    if(dict_cache->dict1 == NULL){
        printf("ERROR 11: dict1 cannot be created!");
        exit(1);
    }

    dict_cache->dict2 = (DICTIONARY*)realloc(dict_cache->dict2, counter2*sizeof(DICTIONARY));
    if(dict_cache->dict2 == NULL){
        printf("ERROR 12: dict1 cannot be created!");
        exit(1);
    }

    // deleted values are not imported into the new dictionaries
    j = 0;
    for(i=0; i<counter1; i++){
        if(strcmp(dictionary1[i].word, "") != 0){
            dict_cache->dict1[j] = dictionary1[i];
            j++;
        }
    }

    j = 0;
    for(i=0; i<counter2; i++){
        if(strcmp(dictionary2[i].word, "") != 0){
            dict_cache->dict2[j] = dictionary2[i];
            j++;
        }
    }

    // the lengths of dictionaries are updated
    dict_cache->num1 = counter1;
    dict_cache->num2 = counter2;

    // words are sorted by frequencies
    qsort(dict_cache->dict1, dict_cache->num1, sizeof(DICTIONARY), compare_frequencies);
    qsort(dict_cache->dict2, dict_cache->num2, sizeof(DICTIONARY), compare_frequencies);

    // updated pointers to skipthe first deleted part
    dict_cache->dict1 = &dict_cache->dict1[(int)(thresholdExtreme*dict_cache->num1)];
    dict_cache->dict2 = &dict_cache->dict2[(int)(thresholdExtreme*dict_cache->num2)];

    // the lengths of dictionaries are updated
    dict_cache->num1 = (int)((1-2*thresholdExtreme)*counter1);
    dict_cache->num2 = (int)((1-2*thresholdExtreme)*counter2);

    return dict_cache;
}

int compare_frequencies(const void *a, const void *b){
    DICTIONARY *dictionary1 = (DICTIONARY*)a;
    DICTIONARY *dictionary2 = (DICTIONARY*)b;
    return dictionary1->frequency - dictionary2->frequency;
}

void genetic_algorithm(DICT_CACHE *dict_cache, int numWord, int numIndiv, float mutatRate){

}