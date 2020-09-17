#include<stdio.h>
#include<stdlib.h>


static int finance_result(int *a,int n);

void boss_read_finance(int *array,int n,int (*fp)(int*,int));
//函数指针的用法
//int main(int argc,char* argv[]){
//  int array[10] = {1,2,3,4,5,6,7,8,9,10};
//  boss_read_finance(array,10,finance_result);
//  return 0;
//}

void boss_read_finance(int* array,int n,int (*fp)(int* a,int n)){
  int result = (*fp)(array,n);
  printf("caculate result:%d\n",result);
}


static int finance_result(int* a,int n){
  int result = 0;
  int i =  0;
  for(i = 0;i < n; i++){
    result += *(a+i);
  }
  return result;
}
