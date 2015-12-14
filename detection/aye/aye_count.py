from __future__ import print_function, division, absolute_import
import cv2
import matplotlib.pyplot as plt
from sklearn.externals import joblib

import rosbag
from cv_bridge import CvBridge, CvBridgeError

from aye.fruit_detector import FruitDetector
from aye.fruit_tracker import FruitTracker
from aye.blob_analysis import region_props, thresh_blobs_area
from aye.preprocessing import rotate_image

# Data to process
im_topic = '/color/image_rect_color'
bagfile = '/home/chao/Workspace/bag/frame/rect_fixed/frame1_rect_fixed.bag'

# Load learning stuff
clf = joblib.load('../model/svc.pkl')
scaler = joblib.load('../model/scaler.pkl')

# Detector and Tracker
detector = FruitDetector(clf, scaler)
tracker = FruitTracker()

# Ros
bridge = CvBridge()

# Visualization
fig = plt.figure()
plt.ion()
ax_bgr = fig.add_subplot(121)
ax_bw = fig.add_subplot(122)
h_bgr = None
h_bw = None

# Main counting loop
with rosbag.Bag(bagfile) as bag:
    for i, (topic, msg, t) in enumerate(bag.read_messages(im_topic)):
        try:
            image = bridge.imgmsg_to_cv2(msg)
            # Rotate image 90 degree
            image = image[200:1000, :1480, :]
            image = rotate_image(image)

        except CvBridgeError as e:
            print(e)
            continue

        # Detection and tracking
        s, bw = detector.detect(image)
        blobs, bw = region_props(bw)

        # This is for debugging purposes, remove later
        # blobs = thresh_blobs_area(blobs, area=50)
        tracker.track(s, blobs)

        # Visualize result
        disp = tracker.disp

        if h_bgr:
            h_bw.set_data(bw)
            h_bgr.set_data(disp)
        else:
            h_bw = ax_bw.imshow(bw, cmap=plt.cm.gray)
            h_bgr = ax_bgr.imshow(disp)
        plt.pause(0.001)
        print(tracker.total_counts)

tracker.finish()
print(tracker.total_counts)