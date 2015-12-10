from __future__ import print_function, division, absolute_import
# common
import os
import cv2
import numpy as np
import matplotlib.pyplot as plt
# scikit learn
from sklearn.preprocessing import StandardScaler
from sklearn.cross_validation import train_test_split
# apple yield estimation
from aye.preprocessing import resize_image, convert_image_colorspace
from aye.tune import tune_svc, print_grid_search_report

cwd = os.getcwd()
data_dir = os.path.join(cwd, '..', 'data')

# Get all file names
file_ids = range(5)
file_ext = 'png'

k = 0.25
v_thresh = 20
test_size = 0.3
X = None
y = None
for fid in file_ids:
    # Get file name
    # f_raw, f_pos, f_neg = get_file_name(fid, file_ext)

    image_name_bgr = "frame{0:04d}.{1}".format(fid, file_ext)
    image_name_pos = "frame{0:04d}_positive.{1}".format(fid, file_ext)
    image_name_neg = "frame{0:04d}_negative.{1}".format(fid, file_ext)
    filename_bgr = os.path.join(data_dir, image_name_bgr)
    filename_pos = os.path.join(data_dir, image_name_pos)
    filename_neg = os.path.join(data_dir, image_name_neg)

    # Load all images
    im_bgr = cv2.imread(filename_bgr, cv2.IMREAD_COLOR)
    im_pos = cv2.imread(filename_pos, cv2.IMREAD_GRAYSCALE)
    im_neg = cv2.imread(filename_neg, cv2.IMREAD_GRAYSCALE)

    # Resize images
    im_bgr = resize_image(im_bgr, k)
    im_pos = resize_image(im_pos, k)
    im_neg = resize_image(im_neg, k)

    # Convert image to different colorspaces
    im_hsv = convert_image_colorspace(im_bgr, 'hsv')
    im_lab = convert_image_colorspace(im_bgr, 'lab')

    # Filter data based on v value in hsv
    v = im_hsv[:, :, -1]
    mask_v = (v > v_thresh) & (v < 255 - v_thresh)
    mask_pos = im_pos > 0
    mask_neg = im_neg > 0

    mask_pos_v = mask_pos & mask_v
    mask_neg_v = mask_neg & mask_v

    # Extract data from images
    X_pos_bgr = im_bgr[mask_pos_v]
    X_pos_hsv = im_hsv[mask_pos_v]
    X_pos_lab = im_lab[mask_pos_v]

    X_neg_bgr = im_bgr[mask_neg_v]
    X_neg_hsv = im_hsv[mask_neg_v]
    X_neg_lab = im_lab[mask_neg_v]

    X_pos = np.hstack((X_pos_bgr, X_pos_hsv, X_pos_lab))
    X_neg = np.hstack((X_neg_bgr, X_neg_hsv, X_neg_lab))

    # Convert to float
    X_pos = np.array(X_pos, float)
    X_neg = np.array(X_neg, float)

    # Make sure we don't have too many negative samples than positive samples
    if len(X_neg) / len(X_pos) > 2:
        X_neg = X_neg[::2]

    # Generate corresponding labels
    y_pos = np.ones((np.size(X_pos, axis=0)))
    y_neg = np.zeros((np.size(X_neg, axis=0)))
    print(X_pos.shape, X_neg.shape)

    X_both = np.vstack((X_pos, X_neg))
    y_both = np.hstack((y_pos, y_neg))

    # Append X_pos, y_pos to X, y
    # TODO: this is bad
    if X is None or y is None:
        X = X_both
        y = y_both
    else:
        X = np.vstack((X, X_both))
        y = np.hstack((y, y_both))

# Pre-processing data
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Split data into train and test
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y,
                                                    test_size=test_size)

# For now, train an SVC with GridSearchCV
svc_grid = tune_svc(X, y)

# Optionally, print a verbose report
print_grid_search_report(svc_grid)

# Validate on y_test

# Load the final image and see visually how good the classifier is

# Maybe calculate how good it is doing