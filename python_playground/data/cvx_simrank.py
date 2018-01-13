# convex optimization for SimRank
import math

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
from sklearn import preprocessing

from simrank import adj_mat

G = [
    (1, 2),
    (2, 1)
]

A = adj_mat(G)
A = A.toarray()
P = preprocessing.normalize(A, 'l1', axis=0)


def f(R, P, c=0.6):
    '''
    the obj function for 2d case
    R: (x,y)
    P: the normalized adj matrix
    '''
    n = P.shape[0]
    y = np.dot(R, R.T) - c * np.dot(P.T, np.dot(R, np.dot(R.T, P))) + \
        c * np.diag(np.dot(P.T, np.dot(R, np.dot(R.T, P)))) - np.eye(n)
    return math.pow(np.linalg.norm(y, "fro"), 2)


@np.vectorize
def compute_z(x, y):
    if x < 0 or y < 0:
        return -2
    R = np.matrix([x, y]).T
    return f(R, P)


def draw_3D_dplot(P):
    fig = plt.figure()
    ax = fig.gca(projection='3d')

    # Make data.
    X = np.arange(0, 2, 0.1)
    Y = np.arange(0, 2, 0.1)
    X, Y = np.meshgrid(X, Y)
    # R = np.sqrt(X**2 + Y**2)
    # Z = np.sin(R)
    Z = compute_z(X, Y)

    # Plot the surface.
    surf = ax.plot_surface(X, Y, Z, cmap=cm.coolwarm,
                           linewidth=0, antialiased=False)

    # Customize the z axis.
    ax.set_zlim(-1.01, 1.01)
    ax.zaxis.set_major_locator(LinearLocator(10))
    ax.zaxis.set_major_formatter(FormatStrFormatter('%.02f'))

    # Add a color bar which maps values to colors.
    fig.colorbar(surf, shrink=0.5, aspect=5)

    plt.show()


def test():
    A = adj_mat(G)
    A = A.toarray()
    P = preprocessing.normalize(A, 'l1', axis=0)
    # print(type(P), P.shape)
    # y = f(np.matrix([1,2]).T, P)
    # print(y)
    draw_3D_dplot(P)


if __name__ == "__main__":
    test()
