#include "cachelab.h"
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>




typedef struct {
    bool valid_bit;
    int tag;
} Line;

typedef struct {
    Line * lines;
} Set;



int main(int argc, char *argv[])
{
    Set *sets;
    int line_number;
    int s_bits = 0;
    int total_set = 0;
    int b_bits = 0;
    int B = 0;
    int hits = 0;
    int misses = 0;
    int evcitions = 0;
    FILE *file;

    int c;
    while( (c = getopt(argc, argv, "s:E:b:t:")) != -1) {
        // -1 means getopt() parse all options
        switch(c) {
            case 's':
                s_bits = atoi(optarg);
                if(!s_bits) return 0;
                total_set = 2<<s_bits;
                break;
            case 'E':
                total_set = atoi(optarg);
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
    sets = malloc(sizeof(Set) * total_set);

    char operation;
    int address;
    while (fscanf(file, "%c %p", &operation, &address) != EOF) {
        printf("%c %p", operation, address);
      // Ignore instruction load
      if (operation == 'I')  continue; 
  
    //   simulate(addr);
      if (operation == 'M') break;
    }
    

    printSummary(0, 0, 0);
    fclose(file);
    free(sets);
    return 0;
}
