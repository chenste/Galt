from __future__ import (print_function, division, absolute_import)

import os
import numpy as np
from sklearn.externals import joblib
from scpye.image_pipeline import ImagePipeline


class FruitDetector(object):
    def __init__(self, img_ppl, img_clf):
        """
        :param img_ppl: image pipeline
        :type img_ppl: ImagePipeline
        :param img_clf: classifier
        """
        self.ppl = img_ppl
        self.clf = img_clf

    def detect_image(self, image):
        # TODO: what else to return from ppl?
        Xt = self.ppl.transform(image)
        y = self.clf.predict(Xt)
        bw = np.array(self.ppl.named_steps['remove_dark'].mask, copy=True)
        bw[bw > 0] = y
        return bw

    def detect_bw(self, bw):
        pass

    def detect(self, image):
        bboxes = []
        gray = []
        return bboxes, gray

    @classmethod
    def from_pickle(cls, model_dir):
        """
        Constructor from a pickle
        :param model_dir:
        :return:
        :rtype: FruitDetector
        """
        img_ppl_file = os.path.join(model_dir, 'img_ppl.pkl')
        img_clf_file = os.path.join(model_dir, 'img_clf.pkl')
        img_ppl = joblib.load(img_ppl_file)
        img_clf = joblib.load(img_clf_file)
        return cls(img_ppl, img_clf)
