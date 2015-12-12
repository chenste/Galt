from __future__ import print_function, division, absolute_import
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches


class OverlapRatio:
    Union, Min = range(2)


def bbox_intersect(bbox1, bbox2):
    x1, y1, w1, h1 = bbox1
    x2, y2, w2, h2 = bbox2
    return (abs(x1 - x2) * 2 < (w1 + w2)) and (abs(y1 - y2) * 2 < (h1 + h2))


def bbox_overlap_ratio(bbox1, bbox2, ratio_type=OverlapRatio.Union):
    if bbox_intersect(bbox1, bbox2):
        x1, y1, w1, h1 = bbox1
        x2, y2, w2, h2 = bbox2
        # intersection area is a * b, where a is width and b is height
        a = max(0, min(x1 + w1, x2 + w2) - max(x1, x2))
        b = max(0, min(y1 + h1, y2 + h2) - max(y1, y2))
        area_intersection = a * b
        area_b1 = w1 * h1
        area_b2 = w2 * h2
        if ratio_type == OverlapRatio.Union:
            area_union = area_b1 + area_b2 - area_intersection
            return area_intersection / area_union
        elif ratio_type == OverlapRatio.Min:
            area_min = min(area_b1, area_b2)
            return area_intersection / area_min
    else:
        return 0.0


def bboxes_overlap_ratio(bboxes1, bboxes2, ratio_type=OverlapRatio.Union):
    n1 = len(bboxes1)
    n2 = len(bboxes2)

    R = np.zeros((n1, n2))
    for i1, b1 in enumerate(bboxes1):
        for i2, b2 in enumerate(bboxes2):
            R[i1, i2] = bbox_overlap_ratio(b1, b2, ratio_type)
    return R


def bbox_distance_squared(bbox1, bbox2):
    x1, y1, w1, h1 = bbox1
    x2, y2, w2, h2 = bbox2
    cx1 = x1 + w1 / 2.0
    cy1 = y1 + h1 / 2.0
    cx2 = x2 + w2 / 2.0
    cy2 = y2 + h2 / 2.0
    return (cx1 - cx2) ** 2 + (cy1 - cy2) ** 2


def bbox_distance_squared_area_ratio(bbox1, bbox2):
    x1, y1, w1, h1 = bbox1
    x2, y2, w2, h2 = bbox2
    cx1 = x1 + w1 / 2.0
    cy1 = y1 + h1 / 2.0
    cx2 = x2 + w2 / 2.0
    cy2 = y2 + h2 / 2.0
    distance_squared = (cx1 - cx2) ** 2 + (cy1 - cy2) ** 2
    area = w1 * h1 + w2 * h2
    return distance_squared / area


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