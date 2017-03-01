#include <stdio.h>
#include <stdlib.h>

int len(int* arr){
    int count = 0;
    while(*arr != -1){
        count++;
        arr++;
    }
    return count;
}

void merge(int* A, int* B, int* L){
	printf("Ever Merging?\n");
	if (*A == -1){

		while(*L != -1){
			*L = *B;
			L++; B++;
		}
		return;
	}
	else if(*B == -1){
		//printf("B is now pointing at terminator\n");
		while(*L != -1){
			*L = *A;
			L++; A++;
			//printf("A is now pointing at %d\n", *A);
		}
		return;
	}
	if(*A < *B){
		*L = *A;
		merge(A + 1, B, L+1);
	}
	else{
		*L = *B;
		merge(A, B+1, L+1);
	}
}



void printArr(int* arr){
	while (*arr != -1){
		printf("%d ", *arr++);
	}
	printf("\n");
}

void mergeSort(int* lst, int size){
	if(size == 1){
		return;
	}
	int* left = (int*) malloc(((size/2) + 1)* sizeof(int));
	*(left + (size/2)) = -1;
	int leftSize = size/2;
	int* right;
	int rightSize;
	if(size%2 == 0){
		right = (int*) malloc(((size/2)) * sizeof(int));
		*(right + (size/2)) = -1;
		rightSize = size/2;
	}
	else{
        printf("ODD SIZE\n");
		//printf("right will be larger size\n");
		right = (int*) malloc(((size/2) + 1) * sizeof(int));
		*(right + (size/2)+1) = -1;
		rightSize = (size/2) + 1;
	}
	int* lstPtr = lst;
	int* leftPtr = left; int* rightPtr = right;
	while (*leftPtr != -1){
		*(leftPtr++) = *(lstPtr++);
	}
	while (*rightPtr != -1){
		*(rightPtr++) = *(lstPtr++);
	}
	printArr(left);
	printArr(right);

	mergeSort(left, leftSize);
	printf("left side return: ");
	printArr(left);
	mergeSort(right, rightSize);
	printf("right side return: ");
	printArr(right);
	merge(left, right, lst);
	free(left);
	free(right);
	printf("after merging: ");
	printArr(lst);




}

int main(){
	/*
	int arr1[] = {2, 4, 6, 8, 10, 12, -1};
	int arr2[] = {1, 3, 5, 7, 9, 11, -1};
	int* L = (int*) malloc(13*sizeof(int));
	*(L + 13) = -1;
	printArr(arr1);
	printArr(arr2);
	merge(arr1, arr2, L);
	printArr(L);
	*/
	int toSort[] = {9, 1, 2, 5, 3, 12, 11, 500, 12, 13, 14, 17, 16, 15, 18, 19, 20, -1};
	printArr(toSort);
    printf("length of array: %d\n", len(toSort));
	mergeSort(toSort, len(toSort));
	printArr(toSort);
	return 0;
}
