from typing import List, Tuple, Callable, Optional
import numpy as np
from dataclasses import dataclass


@dataclass
class _Node:
    point: np.ndarray  # D-dimensional point
    idx: int
    axis: int
    left: "Optional[_Node]"
    right: "Optional[_Node]"


def dist_l2(point_a: np.ndarray, point_b: np.ndarray) -> float:
    return np.sum((point_a-point_b) ** 2) ** 0.5


class KDTree:
    def __init__(self, data: np.ndarray, dist: Optional[Callable] = None):
        """KDTree for fast generalized N-point problems

        Args:
            data (array-like of shape (n_samples, n_features)): [description]
        """
        super().__init__()
        self.dist = dist
        if self.dist is None:
            self.dist = dist_l2
        self.dim: int = data.shape[1]
        indexes = np.arange(len(data))
        self.__tree: Optional[_Node] = self.__build_tree(data, indexes)

    def draw_tree(self):
        from matplotlib import pyplot as plt
        import matplotlib.lines as mlines

        fig = plt.figure(figsize=(15, 15))

        def line(p1, p2):
            plt.plot([p1[0], p2[0]], [p1[1], p2[1]])

        cur_point = (0, 0)

        def draw(tree):
            if tree is None:
                return
            if axis % 2 == 0:
                x = tree.point[0]
                cur_point[0] = x
            else:
                y = tree.point[1]
                cur_point[1] = y
            line(cur_point)

        if self.dim != 2:
            raise f"Can visualize onyl 2dim tree but {self.dim}"

        plt.tight_layout()
        plt.show()
        plt.savefig('figure.png')

    def __build_tree(self, data: np.ndarray, indexes: np.ndarray, axis: int = 0) -> Optional[_Node]:
        if len(data) == 0:
            return None
        sorter = data[:, axis].argsort()  # sort data over the split-axis
        data = data[sorter]
        indexes = indexes[sorter]
        # print('sort by ', axis)
        # print(data)
        middle_idx = data.shape[0] // 2
        middle = data[middle_idx]
        while (middle_idx+1) < data.shape[0] and data[middle_idx+1, axis] == middle[axis]:
            middle_idx += 1
        next_axis = (axis + 1) % data.shape[1]
        # print(middle, axis)
        return _Node(
            point=middle,
            idx=indexes[middle_idx],
            axis=axis,
            left=self.__build_tree(data[:middle_idx], indexes[:middle_idx], axis=next_axis),
            right=self.__build_tree(data[middle_idx+1:], indexes[middle_idx+1:], axis=next_axis)
        )

    def __neares(self, storage, root: _Node, point: np.ndarray, axis: int):
        if root is None:
            return
        dist = self.dist(root.point, point)
        print(point, root.point, root.idx, dist)
        if storage['best_dist'] is None or storage['best_dist'] > dist:
            storage['best_dist'] = dist
            storage['node'] = root
        if dist <= 0.000001:
            return
        dx = root.point[axis] - point[axis]
        next_axis = (axis + 1) % point.shape[0]

        self.__neares(storage, root.right if dx < 0 else root.left, point, next_axis)
        if dx**2 >= storage['best_dist']:
            return
        self.__neares(storage, root.left if dx < 0 else root.right, point, next_axis)

    def query(self, points: np.ndarray, n: int) -> Tuple[List[float], List[int]]:
        """query the tree for the k nearest neighbors

        Args:
            points (array-like of shape (n_samples, n_features)): An array of points to query
            n (int): The number of nearest neighbours to return

        Returns:
            Tuple[List[float], List[int]]: list of (distances, indices) to the neighbors of the corresponding point.
        """
        storage = {'best_dist': None}
        self.__neares(storage, self.__tree, points, axis=0)
        return [storage['best_dist']], [storage['node'].idx]

    def query_range(self):
        """query all points inside the rectangle
        """
        pass
