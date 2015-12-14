from __future__ import print_function, division, absolute_import

import cv2
import numpy as np
import matplotlib.patches as patches


class Colors:
    prediction = (0, 0, 255)  # blue
    detection = (255, 0, 0)  # red
    correction = (255, 255, 0)  # yellow
    new = (255, 255, 255)  # white
    match = (0, 255, 0)  # green
    optical_flow = (255, 0, 0)  # red


# Functions start with plot calls pyplot subroutines and usually requires ax
def plot_filled_bboxes(ax, bboxes, color, alpha):
    for bbox in bboxes:
        x, y, w, h = bbox
        rect = patches.Rectangle((x, y), w, h, facecolor=color, alpha=alpha)
        ax.add_patch(rect)


def plot_edge_bboxes(ax, bboxes, color):
    for bbox in bboxes:
        x, y, w, h = bbox
        rect = patches.Rectangle((x, y), w, h, edgecolor=color, fill=False)
        ax.add_patch(rect)


# Functions start with draw calls opencv subroutines
def draw_bboxes(image, bboxes, color, thickness=1):
    bboxes = np.atleast_2d(bboxes)
    bboxes = np.array(bboxes, copy=True, dtype=int)
    for bbox in bboxes:
        x, y, w, h = bbox
        cv2.rectangle(image, (x, y), (x + w, y + h),
                      color=color, thickness=thickness)


def draw_optical_flow(image, p1, p2, color=(255, 0, 0)):
    for p1g, p2g in zip(p1, p2):
        a, b = p1g.ravel()
        c, d = p2g.ravel()

        cv2.line(image, (a, b), (c, d), color=color, thickness=1)
        cv2.circle(image, (c, d), 1, color=color, thickness=-1)


def draw_bboxes_matches(image, matches, bboxes1, bboxes2, color, thickness=1):
    matches = np.atleast_2d(matches)
    for pair in matches:
        i1, i2 = pair
        b1 = bboxes1[i1]
        b2 = bboxes2[i2]
        x1, y1, w1, h1 = b1
        x2, y2, w2, h2 = b2
        a = int(x1 + w1 / 2)
        b = int(y1 + h1 / 2)
        c = int(x2 + w2 / 2)
        d = int(y2 + h2 / 2)
        cv2.line(image, (a, b), (c, d), color=color, thickness=thickness)