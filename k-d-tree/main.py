import numpy as np
from kd_tree import KDTree


def main():
    dataset_2d = np.array([[2, 3], [5, 4], [9, 6], [4, 7], [8, 1], [7, 2]])
    kdtree = KDTree(dataset_2d)
    dist, ind = kdtree.query(np.array([9, 2]), n=1)


if __name__ == "__main__":
    main()
