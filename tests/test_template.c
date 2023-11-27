#define SIZE <SIZE>

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int validate_matrix(double **mat, double **correct)
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            if (fabs(correct[i][j] - mat[i][j]) > 1e-5)
            {
                printf("Incorrect multiplication\n");
                return 0;
            }
        }
    }
    printf("Correct multiplication\n");
    return 1;
}

double ***read_matrices()
{
    FILE *file = fopen("test-<SIZE>.txt", "r");
    if (file == NULL)
    {
        printf("Could not open file\n");
        return NULL;
    }

    // Allocate memory for the 3D array
    double ***matrices = malloc(4 * sizeof(double **));
    for (int k = 0; k < 4; k++)
    {
        matrices[k] = malloc(SIZE * sizeof(double *));
        for (int i = 0; i < SIZE; i++)
        {
            matrices[k][i] = malloc(SIZE * sizeof(double));
        }
    }

    // Read and store the values
    for (int k = 0; k < 4; k++)
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                if (k < 3)
                {
                    fscanf(file, "%lf", &matrices[k][i][j]);
                }
                else
                {
                    matrices[k][i][j] = 0.0;
                }
            }
        }
    }

    fclose(file);

    return matrices; // Return the matrices
}

void free_matrices(double ***matrices)
{
    for (int k = 0; k < 4; k++)
    {
        for (int i = 0; i < SIZE; i++)
        {
            free(matrices[k][i]);
        }
        free(matrices[k]);
    }
    free(matrices);
}

int main()
{
    double ***matrices = read_matrices();
    double **mat1 = matrices[0];
    double **mat2 = matrices[1];
    double **mat3 = matrices[2];
    double **product = matrices[3];
    clock_t start = clock();

    for (int i = 0; i < SIZE; ++i)
    {
        for (int k = 0; k < SIZE; ++k)
        {
            double tmp = mat1[i][k];
            for (int j = 0; j < SIZE; ++j)
            {
                product[i][j] += tmp * mat2[k][j];
            }
        }
    }

    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Elapsed Time: %0.5f\n", time);

    validate_matrix(mat3, product);
    free_matrices(matrices);

    return 0;
}
