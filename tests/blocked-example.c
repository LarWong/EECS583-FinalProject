#define SIZE 10
#define MAT1 {{-1.275, 7.121, -2.096, 4.56, 2.634, -1.633, -8.522, -2.831, -0.394, -9.197}, {-6.07, 0.68, 1.687, 4.39, 4.272, -8.762, -8.3, 9.055, -5.164, -5.354}, {3.927, -1.281, -5.882, -9.982, 4.477, -4.179, 2.954, 8.137, 7.348, 8.059}, {-3.736, 8.128, -0.371, -4.531, 1.221, 1.417, -0.662, -0.934, -7.693, -1.531}, {-5.971, 5.493, -5.726, 5.273, 1.107, -0.422, -0.151, 3.758, -5.347, -2.344}, {-4.107, 5.194, -4.562, 5.557, -7.742, -0.004, -3.692, -7.193, -4.231, 5.598}, {-9.876, -1.371, 0.075, 9.922, 9.168, -7.285, 6.379, -3.116, -3.986, 3.797}, {-6.88, -2.034, 6.046, 4.409, 2.143, 5.916, 1.184, -9.485, 9.714, -3.481}, {-7.387, 8.826, 5.253, -7.792, -7.79, -9.909, 7.695, -3.799, -3.599, -1.006}, {-9.988, -8.294, 3.212, 5.821, 4.891, 2.874, -5.08, 0.759, 6.389, 2.835}}
#define MAT2 {{8.857, -2.747, 1.469, 5.345, 6.415, 3.633, 0.337, -0.732, 7.75, -5.855}, {8.974, 4.469, -3.879, -7.161, 0.846, -2.86, -5.026, -9.831, 0.006, -7.827}, {7.953, 6.366, -3.391, 5.636, -8.284, -9.807, -7.749, -2.716, 8.082, 7.238}, {7.639, 2.829, -9.48, 2.494, -2.418, -8.257, 3.807, 2.139, 4.265, 5.248}, {4.109, -3.429, 8.546, 1.91, 7.044, 3.135, -6.651, -8.259, -3.081, -9.102}, {5.912, 2.292, 6.257, -1.195, 9.593, 0.675, 5.568, -6.893, -2.242, 9.532}, {-6.172, -3.823, -4.391, -8.294, -3.43, -0.138, -9.067, -0.436, -3.027, -2.676}, {3.36, -1.159, -4.486, 9.282, -7.215, 2.756, -7.42, -5.618, 9.868, -9.568}, {3.477, -0.258, 4.595, 8.763, -8.737, 0.334, 3.887, 6.513, -6.803, -7.627}, {6.104, -3.597, 9.492, -5.106, -6.596, -4.453, -3.721, -4.009, 5.072, -2.457}}
#define MAT3 {{57.521539000000004, 91.152452, -92.312501, 36.64498900000001, 120.832867, -0.7428229999999914, 101.736278, -10.199810999999997, -57.89118, -3.557387000000001}, {-3.9386890000000045, 49.96757500000001, -155.93186699999998, 136.747174, -73.34576899999999, -21.093813000000004, -71.10522499999999, -31.740640000000003, 114.25568, -68.823246}, {-22.20718999999999, -158.733981, 198.21191500000003, 59.930399, -97.77953099999999, 157.83659399999993, -126.29167100000001, -35.27935200000001, -1.868346999999995, -350.11341100000004}, {-19.463719999999995, 41.57292700000001, -16.288741, -153.701236, 105.454862, 10.781538999999993, -67.989219, -144.13829499999997, -20.809775, 7.329942000000006}, {-26.136039999999994, 20.68567300000001, -116.84401199999999, -86.50572900000002, 40.35836800000001, -2.571486999999994, -13.464321000000007, -75.50853799999999, -10.467128000000002, -24.915698999999993}, {2.645617000000008, 51.117921, -47.405345999999994, -187.58448199999998, 12.432375000000008, -100.849516, 130.058473, 32.210775999999996, -23.733786000000002, 147.18394999999998}, {-69.30544299999997, -31.990025, -67.04349600000002, -127.731804, -84.05007500000002, -118.502985, -125.12482699999998, -10.234563, -49.22016299999999, 2.096224000000001}, {19.706198000000015, 83.46406400000001, 39.75929399999998, 24.924588000000018, -32.291897000000006, -111.72311299999998, 106.88061899999998, 89.557421, -186.45222399999997, 182.03585900000002}, {-173.47175899999996, 54.665175, -160.439061, -221.037535, -175.42942800000003, -78.62148499999999, -172.40852999999998, 18.925518000000014, -43.15719199999999, -6.571100000000015}, {17.629183000000012, 23.797622, 86.37486700000001, 135.22511500000005, -112.322782, -82.553895, 73.776426, 60.56846599999999, -54.40176899999999, 110.70849799999998}}

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
  int B = 2;

  for (int kk = 0; kk < SIZE; kk += B) {
    for (int jj = 0; jj < SIZE; jj += B) {
      for (int i = 0; i < SIZE; i += 1) {
        int k_block = kk+B;
        int k_bounds = k_block > SIZE ? SIZE : k_block;
        for (int k = kk; k < k_bounds; k += 1) {
          double r = mat1[i][k];
          int j_block = jj+B;
          int j_bounds = j_block > SIZE ? SIZE : j_block;
          for (int j = jj; j < j_bounds; j += 1) {
            product[i][j] += r*mat2[k][j];
          }
        }
      }
    }
  }

  if (validate_matrix(product) != 1) {
    printf("Incorrect multiplication\n");
    exit(1);
  }

  return 0;
}
