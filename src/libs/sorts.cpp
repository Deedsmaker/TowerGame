#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <string.h>

#include "String.cpp"

void print_arr(int *arr, int count){
    printf("[");
    for (int i = 0; i < count; i++){
        printf("%d", arr[i]);
        if (i < count - 1){
            printf(",");
        }
    }
    
    printf("]\n");
}

void bubble_sort(int *arr, int count){
    for (int i = 0; i < count; i++){
        for (int j = 0; j < count; j++){
            if (arr[i] < arr[j]){
                int temp = arr[j];
                arr[j] = arr[i];
                arr[i] = temp;
            }
        }
    }
}
                                  
#define swap(arr, i, j) int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
                                
int partition(int *arr, int low, int high){
    int pivot = arr[low];
    
    int i = low - 1;
    int j = high + 1;
    
    while (1){
        do {i++;} while (arr[i] < pivot);
        do {j--;} while (arr[j] > pivot);
        
        if (i >= j) return j;
        
        swap(arr, i, j);
    }
}

void q_sort(int *arr, int low, int high){
    if (low < 0 || high < 0 || low >= high) return;
    
    int partition_index = partition(arr, low, high);
    q_sort(arr, low, partition_index);
    q_sort(arr, partition_index + 1, high);
}

inline void sort(int *arr, int arr_count){
    q_sort(arr, 0, arr_count - 1);
}

// int main(){
//     const int count = 10000;
    
//     printf("NULL: %p\n", NULL);
// }
