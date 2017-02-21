import numpy as np

func = min
print min(1, 2)

my_arr = np.ndarray(shape=(2, 3), dtype=float)
my_arr.fill(0)
print my_arr
for i in range(2):
    for j in range(3):
        my_arr[i][j] = i * 3 + j

my_arr /= 2
print my_arr
