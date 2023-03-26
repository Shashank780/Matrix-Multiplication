import numpy as np
import sys

LONG_LONG_MAX =  9223372036854775807

def genMatrix():
    # size = input("Enter nxm matrix size")
    # size = size.split("x")

    n = int(sys.argv[1])
    m = int(sys.argv[2])
    m2 = int(sys.argv[3])

    print(sys.argv)
    #f =  open("Matrix_out.txt")
    mat = np.random.randint(0,1000,size=(n,m))
    # print(mat)
    np.savetxt("matrix1.txt",mat,fmt='%i')

    # size2 = input("Enter second matrix size")
    # size2 = size2.split("x")

    mat2 = np.random.randint(0,1000,size=(m,m2))
    # print(mat2)
    np.savetxt("matrix2.txt",mat2,fmt='%i')


    matRes = np.matmul(mat,mat2)
    print(matRes)
    print(matRes.shape)
    if (matRes>LONG_LONG_MAX).sum() > 0:
        genMatrix()
    np.savetxt("matrixres.txt",matRes,fmt='%i')

if __name__ == "__main__":
    genMatrix()