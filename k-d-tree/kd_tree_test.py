from kd_tree import KDTree, dist_l2
import numpy as np

dataset_2d = np.array([[2, 3], [5, 4], [9, 6], [4, 7], [8, 1], [7, 2]])
dataset_3d = np.array([[2, 3, 1], [9, 4, 5], [4, 6, 7], [1, 2, 5], [7, 8, 9], [3, 6, 1]])


def test_should_find_closest_1():
    kdtree = KDTree(dataset_2d)
    dist, ind = kdtree.query(np.array([9, 2]), n=1)
    assert ind == [4]


def test_should_find_closest_2():
    kdtree = KDTree(dataset_2d)
    dist, ind = kdtree.query(np.array([7, 1]), n=1)
    assert ind == [5]


def test_should_find_closest_3():
    kdtree = KDTree(dataset_2d)
    dist, ind = kdtree.query(np.array([3, 2]), n=1)
    assert ind == [0]


def test_should_find_closest_4():
    kdtree = KDTree(dataset_3d)
    dist, ind = kdtree.query(np.array([1, 2, 3]), n=1)
    assert ind == [3]


def test_should_find_closest_5():
    kdtree = KDTree(dataset_3d)
    dist, ind = kdtree.query(np.array([4, 5, 6]), n=1)
    assert ind == [2]


def test_should_find_closest_6():
    kdtree = KDTree(dataset_3d)
    dist, ind = kdtree.query(np.array([8, 8, 8]), n=1)
    assert ind == [4]


def test_dist():
    assert dist_l2(np.array([0, 0]), np.array([0, 1])) == 1
    assert dist_l2(np.array([0, -1]), np.array([0, 1])) == 2
    assert dist_l2(np.array([0, 0]), np.array([4, 3])) == 5
