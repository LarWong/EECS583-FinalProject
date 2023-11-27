import numpy as np
import sys
import re

SIZE = int(sys.argv[1])

def format_array(mat):
    list_mat = mat.tolist()
    fmt_mat = '{' + ', '.join('{'+', '.join(map(str,row))+'}' for row in list_mat) + '}'
    return fmt_mat

mat1 = np.around(np.random.uniform(low=-10.0, high=10.0, size=(SIZE, SIZE)), 3)
mat2 = np.around(np.random.uniform(low=-10.0, high=10.0, size=(SIZE, SIZE)), 3)
mat3 = np.matmul(mat1, mat2)

with open(f'test-{SIZE}.c', 'x') as code:
    c_template = open('test_template.c', 'r').read()
    res_str = re.sub('<SIZE>', f'{SIZE}', c_template) 
    with open(f'test-{SIZE}.txt', 'x') as data:
        for mat in [mat1, mat2, mat3]:
            for i in mat:
                for j in i:
                    data.write(f'{j}\n')
        data.close()
    code.write(res_str)
    data.close()

