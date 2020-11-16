#include "cachelab.h"
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

typedef struct {
    long long int updated_at;
    bool is_empty;
    int tag;
} Line;

typedef struct {
    Line * lines;
} Set;

const int m = 32;
Set *sets;
int t_bits = 0;
int s_bits = 0;
int line_number;  
int total_set = 0;
int b_bits = 0;
int B = 0;
int hits = 0;
int misses = 0;
int evictions = 0;
long long int tick = 0;

void access_cache(unsigned address);

int main(int argc, char *argv[])
{
    FILE *file;

    int c;
    while( (c = getopt(argc, argv, "s:E:b:t:")) != -1) {
        // -1 means getopt() parse all options
        switch(c) {
            case 's':
                s_bits = atoi(optarg);
                if(!s_bits) return 0;
                total_set = 2 << s_bits;
                break;
            case 'E':
                line_number = atoi(optarg);
                if(!total_set) return 0;
                break;
            case 'b':
                b_bits = atoi(optarg);
                if(!b_bits) return 0;
                break;
            case 't':
                file = fopen(optarg, "r");
                if(!file) return 0;
                break;
            default:
                return 0;
        }
    }
    t_bits = m - s_bits - b_bits;
    sets = malloc(sizeof(Set) * total_set);
    for(int i = 0; i < total_set; i++){
        sets[i].lines = malloc(sizeof(Line) * line_number);
        for(int j = 0; j < line_number; j++)
            sets[i].lines[j].is_empty = true;
    }
        
        

    char operation;
    unsigned address;
    int size;
    while (fscanf(file, " %c %x,%d", &operation, &address, &size) != EOF) {
        tick++;
        if (operation == 'I')  continue; 
        access_cache(address);
        if (operation == 'M'){
            access_cache(address);
        };
    }
    

    printSummary(hits, misses, evictions);
    fclose(file);
    for(int i = 0; i < total_set; i++) free(sets[i].lines);
    free(sets);
    return 0;
}

void access_cache(unsigned address){
    int tag = address >> (s_bits + b_bits);
	int set_index = (address << t_bits) >> (t_bits + b_bits);
    int miss_index = -1;
    int eviction_index = 0;
    Set *selected_set = &sets[set_index];

    for(int i = 0; i < line_number; i++){
        if(!selected_set->lines[i].is_empty && selected_set->lines[i].tag == tag){
            hits++;
            selected_set->lines[i].updated_at = tick;
            return;
        }
        if(miss_index == -1 && selected_set->lines[i].is_empty)
            miss_index = i;
        if(i > 0 && !selected_set->lines[i].is_empty &&
         selected_set->lines[eviction_index].updated_at > selected_set->lines[i].updated_at)
            eviction_index = i;
    }
    misses++;
    if(miss_index !=-1){
        selected_set->lines[miss_index].is_empty = false;
        selected_set->lines[miss_index].tag = tag;
        selected_set->lines[miss_index].updated_at = tick;
        return;
    }
    ++evictions;
    selected_set->lines[eviction_index].is_empty = false;
    selected_set->lines[eviction_index].tag = tag;
    // tick -= selected_set->lines[eviction_index].updated_at;
    // for(int i = 0; i < line_number; i++){
    //     selected_set->lines[i].updated_at -= selected_set->lines[eviction_index].updated_at; 
    // }
    selected_set->lines[eviction_index].updated_at = tick;
    return;
}
