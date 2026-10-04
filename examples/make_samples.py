"""產生範例圖像 (需要 opencv-python 與 numpy)：python3 examples/make_samples.py"""
import cv2
import numpy as np

img = np.zeros((400, 400), np.uint8)
cv2.circle(img, (200, 200), 150, 255, -1)
cv2.imwrite("examples/circle.png", img)

img = np.zeros((400, 400), np.uint8)
box = cv2.boxPoints(((200, 200), (300, 120), 30)).astype(np.int32)
cv2.fillPoly(img, [box], 255)
cv2.imwrite("examples/rotated_rect.png", img)
