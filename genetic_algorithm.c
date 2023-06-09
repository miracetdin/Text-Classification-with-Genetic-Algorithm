#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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

// define a nucleotide (word list) structure
typedef struct nucleotide NUCLEOTIDE;
struct nucleotide{
    char *word;
    int frequency;
};

// define an indivual structure
typedef struct individual INDIVIDUAL;
struct individual{
    NUCLEOTIDE *nuc_codes;
    float fitness;
};

// define a population structure
typedef struct population POPULATION;
struct population{
    INDIVIDUAL *individuals;
    int numIndiv;
    int numWord;
};

// define an order structure
typedef struct order ORDER;
struct order{
    int index;
    float fitness;
    float chance;
    int intervalStart;
    int intervalEnd;
};

// function prototypes
FILE *open_file(char *fileName);
CACHE *read_file(char *fileName);
CACHE *copy_cache(CACHE *cache);
void print_dataSet(CACHE *cache);
DICT_CACHE *create_dictionary(CACHE *cache);
void print_dictionaries(DICT_CACHE *dict_cache);
DICT_CACHE *filter_dicts(DICT_CACHE *dict_cache, float thresholdSame, float thresholdExtreme);
int compare_frequencies(const void *a, const void *b);
void genetic_algorithm(DICT_CACHE *dict_cache, int numWord, int numIndiv, float mutationRate, int Nloop, CACHE *cache2);
INDIVIDUAL *create_individual(DICT_CACHE *dict_cache, int numWord);
void print_individual(INDIVIDUAL *individual, int numWord);
void *fitness_function(DICT_CACHE *dict_cache, POPULATION *population, CACHE *cache2);
INDIVIDUAL *random_selection(ORDER *order, POPULATION *population);
int compare_fitness(const void *a, const void *b);
INDIVIDUAL *reproduce(DICT_CACHE *dict_cache, POPULATION *population, INDIVIDUAL *parent1, INDIVIDUAL *parent2, int numWord);
INDIVIDUAL *mutation(INDIVIDUAL *individual, POPULATION *population, DICT_CACHE *dict_cache);

int main(void){
    CACHE *cache, *cache2;
    DICT_CACHE *dict_cache;

    char *fileName = "amazon_reviews.csv";
    int numWord=10, numIndiv=4;
    float thresholdSame=0.33, thresholdExtreme=0.10, mutationRate=0.0;
    int i, Nloop=200;

    // reading the file
    cache = read_file(fileName);
    // copy the cache
    // cache1 for the tokenization
    // cache2 for the hold the text
    cache2 = copy_cache(cache);

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
    printf("\n\n.....FILTERED DICTIONARIES.....\n\n");
    print_dictionaries(dict_cache);

    // genetic algorithm steps
    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n\n.....GENETIC ALGORITHM.....\n\n");
    genetic_algorithm(dict_cache, numWord, numIndiv, mutationRate, Nloop, cache2);

    free(cache);
    free(cache2);
    free(dict_cache);
    free(fileName);

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
    FILE *file;
    DATA *dataSet;

    char c;
    int lines=0, counter=0, length=0, i;
    char *line, *token;

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
        // read the class
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

    // cache holds the csv file content
    cache->dataSet = dataSet;
    cache->num = lines;
    cache->length = length;

    fclose(file);

    return cache;
}   

CACHE *copy_cache(CACHE *cache){
    CACHE *cache2;

    int i;

    // define a cache which for the transfer the text
    cache2 = (CACHE*)malloc(sizeof(CACHE));
    if(cache2 == NULL){
        printf("ERROR 4: cache2 cannot be created!");
        exit(1);
    }

    cache2->dataSet = (DATA*)malloc((cache->num)*sizeof(DATA));
    if(cache2->dataSet == NULL){
        printf("ERROR 4: cache2->dataSet cannot be created!");
        exit(1);
    }

    for(i=0; i<cache->num; i++){
        cache2->dataSet[i].class = (char*)malloc(cache->length*sizeof(char));
        cache2->dataSet[i].text = (char*)malloc(cache->length*sizeof(char));
    }

    // copy the text
    cache2->length = cache->length;
    cache2->num = cache->num;
    for(i=0; i<cache->num; i++){
        memcpy(cache2->dataSet[i].class, cache->dataSet[i].class, sizeof(char));
        memcpy(cache2->dataSet[i].text, cache->dataSet[i].text, sizeof(char)*cache->length);
    }

    return cache2;
}

void print_dataSet(CACHE *cache){
    int i=0;
    
    printf("\nData Set Info:\n");
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

    char *class1, *token;
    int i, j, same=0, counter1, counter2=0;
    
    // define two dictionaries' pointers
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

    // assign the class1 from the first data of data set
    class1 = cache->dataSet[1].class;

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
    
    // dictionary cache points to dictionaries
    dict_cache->dict1 = dictionary1;
    dict_cache->num1 = counter1;
    dict_cache->dict2 = dictionary2;
    dict_cache->num2 = counter2;

    free(class1);
    free(token);

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

    int i, j, counter1, counter2;
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

    // updated pointers to skip the first deleted part
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

void genetic_algorithm(DICT_CACHE *dict_cache, int numWord, int numIndiv, float mutationRate, int Nloop, CACHE *cache2){
    INDIVIDUAL *individual, *new_individual;
    POPULATION *population, *new_population;
    INDIVIDUAL *selected_indiv1, *selected_indiv2, *new_indiv;
    ORDER *order;
    INDIVIDUAL *best_indiv;

    int i, j, n, total, mutation_prob, step, best_index_gen;

    float *gen_best_fits;
    float *gen_avg_fits;
    float total_fits = 0.0, gen_max_fit = 0.0, gen_avg_fit = 0.0, best_fit = 0.0, best_avg_fit = 0.0;

    gen_best_fits = (float*)malloc(Nloop*sizeof(float));
    gen_avg_fits = (float*)malloc(Nloop*sizeof(float));

    //--------------------------
    // Before the Genetic Algorithm
    // Population Creating
    //--------------------------

    // define the pointers
    individual = (INDIVIDUAL*)calloc(1, sizeof(INDIVIDUAL));
    if(individual == NULL){
        printf("ERROR 13: individual cannot be created!");
        exit(1);
    }

    population = (POPULATION*)calloc(1, sizeof(POPULATION));
    if(population == NULL){
        printf("ERROR 14: population cannot be created!");
        exit(1);
    }

    for(i=0; i<numIndiv; i++){
        population->individuals = (INDIVIDUAL*)calloc(numWord, sizeof(INDIVIDUAL));
        if(population == NULL){
            printf("ERROR 14: population->individuals cannot be created!");
            exit(1);
        }
    }
    
    // create individual randomly
    srand(time(NULL)); 
    for(i=0; i<numIndiv; i++){
        individual = create_individual(dict_cache, numWord);
        population->individuals[i] = *individual;
    }

    population->numIndiv = numIndiv;
    population->numWord = numWord;

    fitness_function(dict_cache, population, cache2);

    // print the first population
    printf("\nFIRST POPULATION\n");
    printf("----------\n");
    for(i=0; i<population->numIndiv; i++){
        printf("Individual %d: \n", i);
        printf("[");
        print_individual(&(population->individuals[i]), population->numWord);
        printf("]");
        printf("\nfitness: %f\n", population->individuals[i].fitness);
        printf("\n");
    }

    //--------------------------
    // Optimization
    //--------------------------
    // number of generations (#iteration)
    for(step=0; step<Nloop; step++){

        fitness_function(dict_cache, population, cache2);

        // Statistical Results
        gen_max_fit = population->individuals[0].fitness;
        total_fits = 0.0;
        for(i=0; i<population->numIndiv; i++){
            if(population->individuals[i].fitness > gen_max_fit){
                gen_max_fit = population->individuals[i].fitness; 
            }
            if(best_fit < population->individuals[i].fitness){
                best_fit = population->individuals[i].fitness;
                best_indiv = &population->individuals[i];
                best_index_gen = step;
            }
            total_fits += population->individuals[i].fitness;
        }
        gen_avg_fit = (float)(total_fits)/(float)(population->numIndiv);

        gen_avg_fits[step] = gen_avg_fit;
        gen_best_fits[step] = gen_max_fit;

        if(best_avg_fit < gen_avg_fit){
            best_avg_fit = gen_avg_fit;
        }

        //--------------------------
        // 1. New Population Creating
        //--------------------------

        // define the pointers
        new_individual = (INDIVIDUAL*)calloc(1, sizeof(INDIVIDUAL));
        if(new_individual == NULL){
            printf("ERROR 13: individual cannot be created!");
            exit(1);
        }

        new_population = (POPULATION*)calloc(1, sizeof(POPULATION));
        if(new_population == NULL){
            printf("ERROR 14: population cannot be created!");
            exit(1);
        }

        for(i=0; i<numIndiv; i++){
            new_population->individuals = (INDIVIDUAL*)calloc(numWord, sizeof(INDIVIDUAL));
            if(new_population->individuals == NULL){
                printf("ERROR 14: population->individuals cannot be created!");
                exit(1);
            }
        }

        // create individual randomly
        for(i=0; i<numIndiv; i++){
            new_individual = create_individual(dict_cache, numWord);
            new_population->individuals[i] = *new_individual;
        }

        new_population->numIndiv = numIndiv;
        new_population->numWord = numWord;

        //--------------------------
        // 2. Compute the Fitnees Values
        // Order by Fitness Values
        //--------------------------

        fitness_function(dict_cache, new_population, cache2);

        // define an order pointer for Random Selection
        order = (ORDER*)calloc(population->numIndiv, sizeof(ORDER));
        if(order == NULL){
            printf("ERROR 5: order cannot be created!");
            exit(1);
        }

        for(i=0; i<population->numIndiv; i++){
            order[i].index = i;
            order[i].fitness = population->individuals[i].fitness;
        }
        // sorting the individuals by their fitness values
        qsort(order, population->numIndiv, sizeof(ORDER), compare_fitness);
        // compute the chance calue for each individuals
        // only best half part individuals have a chance
        total = (population->numIndiv/2) * (population->numIndiv/2+1) / 2;
        int order_index = population->numIndiv/2;
        for(i=order_index; i<population->numIndiv; i++){
            order[i].chance = ((float)(i-order_index+1)/total) * 100;
        }
        // compute the chance-interval values
        order[order_index].intervalStart = 0;
        order[order_index].intervalEnd = ceil(order[order_index].chance);
        for(i=order_index+1; i<population->numIndiv; i++){
            order[i].intervalStart = ceil(order[i-1].intervalEnd);
            order[i].intervalEnd = ceil(order[i].intervalStart + order[i].chance);
        }
        
        //--------------------------
        // Creating New Population
        //--------------------------
        for(n=0; n<population->numIndiv; n++){
 
            new_indiv = (INDIVIDUAL*)calloc(1, sizeof(INDIVIDUAL));
            if(new_indiv == NULL){
                printf("ERROR 13: individual cannot be created!");
                exit(1);
            }

            //--------------------------
            // 3. Random Selection
            //    Ordered by Fitness Values
            //--------------------------
            
            // Parent Selection
            selected_indiv1 =  random_selection(order, population);
            selected_indiv2 =  random_selection(order, population);
            
            // Crossover
            new_indiv = reproduce(dict_cache, population, selected_indiv1, selected_indiv2, population->numWord);

            //--------------------------
            // 4. Mutation
            //--------------------------
            
            // mutation probability
            mutation_prob = rand() % 100;
            mutationRate = mutationRate - (float)(mutationRate/Nloop);
            if(mutationRate > mutation_prob){
                new_indiv = mutation(new_indiv, population, dict_cache);
            }

            // Add New Child Indiv to New population
            for(i=0; i<population->numWord; i++){
                new_population->individuals[n].nuc_codes[i].word = new_indiv->nuc_codes[i].word;
                new_population->individuals[n].nuc_codes[i].frequency = new_indiv->nuc_codes[i].frequency;
            }

            free(new_indiv);
        }       

        fitness_function(dict_cache, new_population, cache2);
        
        // copy new_population to population
        for(i=0; i<population->numIndiv; i++){
            for(j=0; j<population->numWord; j++){
                population->individuals[i].nuc_codes[j].word = new_population->individuals[i].nuc_codes[j].word;
                population->individuals[i].nuc_codes[j].frequency = new_population->individuals[i].nuc_codes[j].frequency;
            }
            population->individuals[i].fitness = new_population->individuals[i].fitness;
        }

        population->numIndiv = new_population->numIndiv;
        population->numWord = new_population->numWord;

        free(new_population);
    }

    // print the population
    printf("\nLAST POPULATION\n");
    printf("----------\n");
    for(i=0; i<population->numIndiv; i++){
        printf("Individual %d: \n", i);
        printf("[");
        print_individual(&(population->individuals[i]), population->numWord);
        printf("]");
        printf("\nfitness: %f\n", population->individuals[i].fitness);
        printf("\n");
    }

    // Print Fitness Scores
    for(i=0; i<Nloop; i++){
        printf("\n%d gen_avg: %f", i, gen_avg_fits[i]);
        printf("\n%d gen_best: %f", i, gen_best_fits[i]);
    }
    printf("\nmax_fit: %f", best_fit);
    printf("\nmax_fit gen: %d", best_index_gen);
    printf("\nbest indiv: ");
    print_individual(best_indiv, population->numWord);
    printf("\nbest avg: %f\n", best_avg_fit);

    free(individual);
    free(new_indiv);
    free(population);
    free(new_population);
    free(selected_indiv1);
    free(selected_indiv2);
    free(order);
    free(new_individual);
}

INDIVIDUAL *create_individual(DICT_CACHE *dict_cache, int numWord){
    NUCLEOTIDE *nuc_codes;
    INDIVIDUAL *individual;
    int i, j;

    // nucleotid codes (individual's words number)
    nuc_codes = (NUCLEOTIDE*)calloc(numWord, sizeof(NUCLEOTIDE));
    if(nuc_codes == NULL){
        printf("ERROR 15: nuc_codes cannot be created!");
        exit(1);
    }

    // assign random words (nucleotid codes) to individual
    for(i=0; i<numWord; i++){
        // randomly dictionary selection     
        j = rand() % 2;
        if(j == 0){
            // randomly word selection
            j = rand() % dict_cache->num1;
            nuc_codes[i].word = dict_cache->dict1[j].word;
            nuc_codes[i].frequency = dict_cache->dict1[j].frequency;
        }
        else{
            j = rand() % dict_cache->num2;
            nuc_codes[i].word = dict_cache->dict2[j].word;
            nuc_codes[i].frequency = dict_cache->dict2[j].frequency;
        }
    }

    // define an individual
    individual = (INDIVIDUAL*)calloc(1, sizeof(INDIVIDUAL));
    if(individual == NULL){
        printf("ERROR 16: individual cannot be created!");
        exit(1);
    }

    // assign the nucleotid codes to individual
    individual->nuc_codes = nuc_codes;
    individual->fitness = 0;

    return individual;
}

void print_individual(INDIVIDUAL *individual, int numWord){
    int i;

    for(i=0; i<numWord; i++){
        if(i == 0){
            printf("[");
        }
        if(i == (numWord/2)){
            printf("[");
        }
        printf(individual->nuc_codes[i].word);
        if(i == (numWord/2)-1){
            printf("], ");
        }
        if(i == numWord-1){
            printf("]");
        }
        if(i != (numWord/2)-1 && i != numWord-1){
            printf(", ");
        }
    }
}

void *fitness_function(DICT_CACHE *dict_cache, POPULATION *population, CACHE *cache2){
    int i, j, k, truePredict, counter1=0, counter2=0;
    char *class;

    // compute every individual's score
    for(i=0; i<population->numIndiv; i++){
        truePredict = 0;
        // control the all the texts in dataset
        for(j=0; j<cache2->num; j++){
            counter1 = 0;
            counter2 = 0;
            // control for class 1
            for(k=0; k<population->numWord/2; k++){
                // TODO: anlamsız kelimeller metinde kelime içinde yer alabiliyor
                // de -> hidden
                // if the word places in the text, increment the score 
                if(strstr(cache2->dataSet[j].text, population->individuals[i].nuc_codes[k].word)){
                    counter1++;
                }
            }
            // control for class 2
            for(k=population->numWord/2; k<population->numWord; k++){
                if(strstr(cache2->dataSet[j].text, population->individuals[i].nuc_codes[k].word)){
                    counter2++;
                }
            }

            // if the class 1 has more points, prediction is class 1
            if(counter1 > counter2){
                class = dict_cache->dict1->class;
            }
            else if(counter1 < counter2){
                class = dict_cache->dict2->class;
            }
            if(counter1 == counter2){
                    if(rand() % 2 == 0){
                        class = dict_cache->dict1->class;
                    }
                    else{
                        class = dict_cache->dict2->class;
                    }
            }

            // control whatever prediction is true or false 
            if(strcmp(class, cache2->dataSet[j].class) == 0){
                truePredict++;
            }
        }

        // compute the fitness value (success rate)
        population->individuals[i].fitness = (float)(truePredict) / cache2->num;
    }
 
}

INDIVIDUAL *random_selection(ORDER *order, POPULATION *population){
    int i, rand_value, selected;

    // select an individual randomly to be a parent
    rand_value = rand() % 100;
    for(i=population->numIndiv/2; i<population->numIndiv; i++){
        if((order[i].intervalStart <= rand_value) && (rand_value < order[i].intervalEnd)){
            selected = i;
            return &(population->individuals[order[i].index]);
        }
    }
}

int compare_fitness(const void *a, const void *b){
    ORDER *order1 = (ORDER*)a;
    ORDER *order2 = (ORDER*)b;
    return (order1->fitness - order2->fitness) * 1000000; 
}

INDIVIDUAL *reproduce(DICT_CACHE *dict_cache, POPULATION *population, INDIVIDUAL *parent1, INDIVIDUAL *parent2, int numWord){
    INDIVIDUAL *child;

    int i, rand_value, rand_class;
    
    // create a child with random nucleotide codes
    child = create_individual(dict_cache, numWord);
    // copy the parent1 codes to child
    for(i=0; i<numWord; i++){
        child->nuc_codes[i].word, parent1->nuc_codes[i].word;
        child->nuc_codes[i].frequency = parent1->nuc_codes[i].frequency;
        child->fitness = parent1->fitness;
    }

    // select a class randomly for crossover
    rand_class = rand() % 2;

    // crossover for class 1 --> individual: class1 + class2
    if(rand_class == 0){
        // crossover with random ratio for class 1 
        rand_value = rand() % numWord/2;
        if(rand_value != 0){
            // copy parent 2's nucleotide codes to child codes 
            // start index: rand value
            for(i=rand_value; i<numWord/2; i++){
                child->nuc_codes[i].word = parent2->nuc_codes[i].word;
            }
        }
    }
    // crossover for class 2
    else{
        rand_value = rand() % numWord/2;
        rand_value += numWord/2;
        if(rand_value != numWord/2){
            for(i=rand_value; i<numWord; i++){
                child->nuc_codes[i].word = parent2->nuc_codes[i].word;
            }
        }
    }

    return child;
}

INDIVIDUAL *mutation(INDIVIDUAL *individual, POPULATION *population, DICT_CACHE *dict_cache){
    int i, rand_class, rand_index, rand_word;

    // dictionary selection
    rand_class = rand() % 2;

    // dictionary 1
    if(rand_class == 0){
        // which index will be mutated
        rand_index = rand() % population->numWord;
        // select the new word
        rand_word = rand() % dict_cache->num1;
        // assigne the new word and it's frequency
        strcpy(individual->nuc_codes[rand_index].word, dict_cache->dict1[rand_word].word);
        individual->nuc_codes[rand_index].frequency = dict_cache->dict1[rand_word].frequency;
    }
    // dictionary 2
    else{
        rand_index = rand() % population->numWord;
        rand_word = rand() % dict_cache->num2;
        strcpy(individual->nuc_codes[rand_index].word, dict_cache->dict2[rand_word].word);
        individual->nuc_codes[rand_index].frequency = dict_cache->dict2[rand_word].frequency;
    }

    return individual;
}