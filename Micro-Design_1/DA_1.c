#include <stdio.h>
#include <stdlib.h>

int main()
{
  int arr_0[30][10];
  int arr_1[30][10];
  int arr_2[30][10];

  int *divArr;
  int *ndivArr;

  int div_arr[300];
  int ndiv_arr[300];
  int a = 0x0222;
  int b, c;
  int sum;
  int i, j;
  int temp;
  int div_count = 0;
  int ndiv_count = 0;

  printf("\nTask #1:\n");
  printf("Summation Array\n");
  for(i = 0; i < 30; i++)
  {
    for(j = 0; j < 10; j++)
    {
      b = (a & 0x0000FF00) >> 8;
      c = a & 0x000000FF;
      sum = b + c;
      sum = sum & 0x000000FF;
      arr_0[i][j] = sum;
      printf("%3x", arr_0[i][j]);
      a++;
    }
    printf("\n");
  }

  printf("\nTask #2:\n");
  for(i = 0; i < 30; i++)
  {
    for(j = 0; j < 10; j++)
    {
      temp = arr_0[i][j] % 0x05;
      if(arr_0[i][j] == 0 || temp != 0)
      {
        arr_2[i][j] = arr_0[i][j];
        arr_1[i][j] = 0x0;
        ndiv_count++;
      }
      else
      {
        arr_1[i][j] = arr_0[i][j];
        arr_2[i][j] = 0x0;
        div_count++;
      }
    }
  }

  divArr = malloc(div_count*sizeof(int));
  temp = 0;
  for(i = 0; i < 30; i++)
  {
    for(j = 0; j < 10; j++)
    {
      if(arr_1[i][j] > 0)
      {
        divArr[temp] = arr_1[i][j];
        temp++;
      }
    }
  }

  ndivArr = malloc(ndiv_count*sizeof(int));
  temp = 0;
  for(i = 0; i < 30; i++)
  {
    for(j = 0; j < 10; j++)
    {
      if(arr_2[i][j] > 0)
      {
        ndivArr[temp] = arr_2[i][j];
        temp++;
      }
    }
  }

  printf("\nNumbers divisible by 5\n");
  temp = 0;
  for(i = 0; i < div_count; i++)
  {
    printf("%3x", divArr[i]);
    temp++;
    if(temp == 10)
    {
      printf("\n");
      temp = 0;
    }
  }
  printf("\nNumbers aren't divisible by 5\n");
  temp = 0;
  for(i = 0; i < ndiv_count; i++)
  {
    printf("%3x", ndivArr[i]);
    temp++;
    if(temp == 10)
    {
      printf("\n");
      temp = 0;
    }
  }
  printf("\nTotal numbers that are divisible by 5: %d", div_count);
  printf("\nTotal numbers that aren't divisible by 5: %d\n", ndiv_count);

  printf("\nTask #3:\n");
  sum = 0;
  for(i = 0; i < div_count; i++)
  {
    sum += divArr[i];
  }
  printf("Sum of Div: %x\n", sum);

  sum = 0;
  for(i = 0; i < ndiv_count; i++)
  {
    sum += ndivArr[i];
  }
  printf("Sum of not Div: %x\n", sum);

  free(divArr);
  free(ndivArr);
  return 0;
}
