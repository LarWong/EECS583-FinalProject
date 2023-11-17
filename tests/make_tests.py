import numpy as np
import sys
import re

TEST_NUM = sys.argv[1]
SIZE = int(sys.argv[2])

def format_array(mat):
    list_mat = mat.tolist()
    fmt_mat = '{' + ', '.join('{'+', '.join(map(str,row))+'}' for row in list_mat) + '}'
    return fmt_mat

mat1 = np.around(np.random.uniform(low=-10.0, high=10.0, size=(SIZE, SIZE)), 3)
mat2 = np.around(np.random.uniform(low=-10.0, high=10.0, size=(SIZE, SIZE)), 3)
mat3 = np.matmul(mat1, mat2)

with open(f'test-{TEST_NUM}.c', 'x') as f:
    c_template = open('test_template.c', 'r').read()
    res_str = re.sub('<SIZE>', f'{SIZE}', c_template) 
    res_str = re.sub('<MAT1>', format_array(mat1), res_str)
    res_str = re.sub('<MAT2>', format_array(mat2), res_str) 
    res_str = re.sub('<MAT3>', format_array(mat3), res_str) 
    f.write(res_str)
    f.close()

