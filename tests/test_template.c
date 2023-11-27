#define SIZE <SIZE>
#define MAT1 <MAT1>
#define MAT2 <MAT2>
#define MAT3 <MAT3>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int validate_matrix(double mat[SIZE][SIZE]) {
  double ans[SIZE][SIZE] = MAT3;
  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      if (fabs(ans[i][j] - mat[i][j]) > 1e-3) {
        return 0;
      }
    }
  }
  printf("Correct multiplication\n");
  return 1;
}

int main() {
  double mat1[SIZE][SIZE] = MAT1;
  double mat2[SIZE][SIZE] = MAT2;
  double product[SIZE][SIZE] = {{0.0}};

  for (int i = 0; i < SIZE; ++i) {
    for (int k = 0; k < SIZE; ++k) {
      double tmp = mat1[i][k];
      for (int j = 0; j < SIZE; ++j) {
        product[i][j] += tmp * mat2[k][j];
      }
    }
  }

  if (validate_matrix(product) != 1) {
    printf("Incorrect multiplication\n");
    exit(1);
  }

  return 0;
}
