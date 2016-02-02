from __future__ import (absolute_import, division, print_function)
import cv2
import numpy as np
from sklearn.base import TransformerMixin, BaseEstimator
from functools import partial

from scpye.bounding_box import extract_bbox

__all__ = ['ImageRotator', 'ImageCropper', 'ImageResizer', 'DarkRemover',
           'CspaceTransformer']


def split_label01(label):
    return label[:, :, 0], label[:, :, 1]


class ImageTransformer(BaseEstimator, TransformerMixin):
    """
    Base class for image transformation
    """

    def fit(self, X, y=None):
        return self

    @staticmethod
    def _transform(X, func):
        """
        Apply transform to X return None if X is None
        :param X: data
        :param func: function to apply to data
        :return: transformed data
        """
        if X is None:
            return None

        return func(X)


class ImageRotator(ImageTransformer):
    def __init__(self, ccw=-1):
        """
        :param ccw: number of counter-clockwise rotations
        :type ccw: int
        """
        self.ccw = ccw

    def transform(self, X, y=None):
        """
        :param X: image
        :param y: label
        :return: rotated image and label
        """
        func = partial(np.rot90, k=self.ccw)
        Xt = self._transform(X, func)
        yt = self._transform(y, func)
        return Xt, yt


class ImageCropper(ImageTransformer):
    def __init__(self, bbox=None):
        self.bbox = bbox

    def transform(self, X, y=None):
        """
        :param X: image
        :param y: label
        :return: region of image and label
        """
        func = partial(extract_bbox, bbox=self.bbox)
        Xt = self._transform(X, func)
        yt = self._transform(y, func)
        return Xt, yt


class ImageResizer(ImageTransformer):
    def transform(self, X, y=None):
        """
        :param X: image
        :param y: label
        :return: resized image
        """
        k = 0.5
        func_x = cv2.pyrDown
        func_y = partial(cv2.resize, dsize=None, fx=k, fy=k,
                         interpolation=cv2.INTER_NEAREST)
        Xt = self._transform(X, func_x)
        yt = self._transform(y, func_y)
        return Xt, yt


class DarkRemover(ImageTransformer):
    def __init__(self, v_min=25):
        """
        :param v_min: minimum value in v channel
        :type v_min: int
        """
        assert 0 < v_min < 255
        self.mask = None
        self.v_min = v_min

    def transform(self, X, y=None):
        """
        :param X: image
        :param y: label
        :return: a tuple of bgr image and mask
        """
        img_hsv = cv2.cvtColor(X, cv2.COLOR_BGR2HSV)
        self.mask = img_hsv[:, :, -1] > self.v_min
        if y is None:
            return (X, self.mask), y

        neg, pos = split_label01(y)

        neg_mask = self.mask & neg
        pos_mask = self.mask & pos

        y_pos = np.ones(np.count_nonzero(pos_mask))
        y_neg = np.zeros(np.count_nonzero(neg_mask))

        mask = np.dstack((neg_mask, pos_mask))
        yt = np.hstack((y_neg, y_pos))
        return (X, mask), yt


class CspaceTransformer(ImageTransformer):
    def __init__(self, des):
        self.des = des
        self.img = None

    def cspace_transform(self, src):
        """
        :param src: bgr image
        :return: bgr image in other colorspace
        """
        if np.ndim(src) == 2:
            src = np.expand_dims(src, 1)

        if self.des == 'hsv':
            des = cv2.cvtColor(src, cv2.COLOR_BGR2HSV)
        elif self.des == 'lab':
            des = cv2.cvtColor(src, cv2.COLOR_BGR2LAB)
        elif self.des == 'bgr':
            des = src
        else:
            raise ValueError("{0} not supported".format(self.des))

        return np.squeeze(des)

    def transform(self, X, y=None):
        """
        :param X: tuple of bgr image and mask
        :return: masked image with transformed colorspace
        """
        if y is None:
            bgr, mask = X
            Xt = self.cspace_transform(bgr[mask])
            self.img = np.zeros_like(bgr)
            self.img[mask] = Xt
        else:
            bgr, label = X
            neg, pos = split_label01(label)
            Xt_neg = self.cspace_transform(bgr[neg])
            Xt_pos = self.cspace_transform(bgr[pos])
            Xt = np.vstack((Xt_neg, Xt_pos))
        # Need to change to float to suppress later warnings
        return np.array(Xt, np.float64)


def xy_from_array(m):
    """
    :param m: array
    :type m: numpy.ndarray
    :return: n x 2 matrix of [x, y]
    """
    assert np.ndim(m) == 2
    r, c = np.where(m)
    return np.hstack((r.T, c.T))


class PixelIndexer(ImageTransformer):
    def transform(self, X, y=None):
        if y is None:
            _, mask = X
            Xt = xy_from_array(mask)
        else:
            _, label = X
            neg, pos = split_label01(label)
            xy_neg = xy_from_array(neg)
            xy_pos = xy_from_array(pos)
            Xt = np.vstack((xy_neg, xy_pos))
        # Change to float to suppress warning
        return np.array(Xt, np.float64)
