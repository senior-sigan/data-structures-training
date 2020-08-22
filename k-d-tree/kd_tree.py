from typing import List, Tuple, Callable, Optional
import numpy as np
from dataclasses import dataclass, field
import heapq


@dataclass
class _Node:
    point: np.ndarray  # D-dimensional point
    idx: int
    axis: int
    left: "Optional[_Node]"
    right: "Optional[_Node]"


@dataclass(order=True)
class _Candidate:
    dist: float
    idx: int = field(compare=False)


def _priority_push(container: List[_Candidate], element: _Candidate, max_size: int) -> None:
    element.dist *= -1  # hack for heapq to reverse order
    if len(container) >= max_size:
        heapq.heappushpop(container, element)
    else:
        heapq.heappush(container, element)


def _get_best_dist(container: List[_Candidate]) -> _Candidate:
    return -heapq.nlargest(1, container)[0].dist


def _get_sorted(container: List[_Candidate]) -> List[_Candidate]:
    return sorted(container, reverse=True)


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

    def __nearest(self, storage: List[_Candidate], root: _Node, point: np.ndarray, axis: int, n: int):
        if root is None:
            return
        dist = self.dist(root.point, point)
        print(point, root.point, root.idx, dist)

        _priority_push(storage, _Candidate(idx=root.idx, dist=dist), max_size=n)

        if dist <= 0.000001:  # why this magic number?
            return
        dx = root.point[axis] - point[axis]
        next_axis = (axis + 1) % point.shape[0]

        self.__nearest(storage, root.right if dx < 0 else root.left, point, next_axis, n)
        best_dist = _get_best_dist(storage)
        if dx**2 >= best_dist:  # what is that?
            return
        self.__nearest(storage, root.left if dx < 0 else root.right, point, next_axis, n)

    def query(self, points: np.ndarray, n: int) -> Tuple[List[float], List[int]]:
        """query the tree for the k nearest neighbors

        Args:
            points (array-like of shape (n_samples, n_features)): An array of points to query
            n (int): The number of nearest neighbours to return

        Returns:
            Tuple[List[float], List[int]]: list of (distances, indices) to the neighbors of the corresponding point.
        """
        storage: List[_Candidate] = []
        self.__nearest(storage, self.__tree, points, axis=0, n=n)
        storage = _get_sorted(storage)
        print(storage)
        return [el.dist for el in storage], [el.idx for el in storage]

    def query_range(self):
        """query all points inside the rectangle
        """
        pass
