#include <stdio.h>

#define arr 5
#define process 5

void first(int arraySize[], int processSize[]){
  int allocation[process] = {0};

  // implementation
  int tempSize[arr];
  for (int i = 0; i < arr ; i++){
    tempSize[i] = arraySize[i];  
  }
  
  for(int p = 0; p < process; p++){
    int m  = processSize[p] + ((4 - processSize[p] % 4) == 4 ? 0 : 4 - processSize[p] % 4);
    for(int b = 0; b < arr; b++){
      if(m < tempSize[b]){
        allocation[p] = b;
        tempSize[b] -= m;
        break;
      }
    }
  }
  //

  for (int i = 0; i < arr ; i++){
    printf("Process %d, Block %d\n", i, allocation[i]);
  }
}

void best(int arraySize[], int processSize[]){
  int allocation[process] = {0};

  // implementation
  int tempSize[arr];
  for (int i = 0; i < arr ; i++){
    tempSize[i] = arraySize[i];  
  }

  for(int p = 0; p < process; p++){
    int pos = -1;
    int m  = processSize[p] + ((4 - processSize[p] % 4) == 4 ? 0 : 4 - processSize[p] % 4);
    int leftOver = 10000;
    for(int b = 0; b < arr; b++){
      if(m < tempSize[b]){
        if(leftOver > tempSize[b] - m){
          
          leftOver = tempSize[b] - m;
          pos = b;
        }
      }
    }
    
    allocation[p] = pos;
    tempSize[pos] -= m;
    
  }
  //

  for (int i = 0; i < arr ; i++){
    printf("Process %d, Block %d\n", i, allocation[i]);
  }
}

int main()
{
  int arraySize[] = {30, 50, 40, 10, 20};
  int processSize[] = {41, 4, 24, 15, 19};

  first(arraySize,processSize);
  best(arraySize,processSize);
}

