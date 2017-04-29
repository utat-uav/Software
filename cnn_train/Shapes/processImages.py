#I am not the author of this code


import os
import cv2
import math
import numpy as np
import time

def rotate_image(image, angle):
    #calculates the length of the diagonal (PYTHAGORAS)
    diagonal = int(math.sqrt(pow(image.shape[0], 2) + pow(image.shape[1], 2)))
    #calculates the difference between the length of the diagonal and the horizontal (offset_x) or the vertical (offset_y)
    offset_x = (diagonal - image.shape[0])/2
    offset_y = (diagonal - image.shape[1])/2

    #dtype = unsigned integer (8-bit?)
    dst_image = np.zeros((diagonal, diagonal, 3), dtype='uint8')

    #not really center? or am I just tripping
    image_center = (diagonal/2, diagonal/2)

    R = cv2.getRotationMatrix2D(image_center, angle, 1.0)
    dst_image[int(offset_x) : int(offset_x + image.shape[0]),\
              int(offset_y) : int(offset_y + image.shape[1]), :] = image
    dst_image = cv2.warpAffine(dst_image, R, (diagonal, diagonal), flags=cv2.INTER_LINEAR)

    # Calculate the rotated bounding rect
    x0 = offset_x
    x1 = offset_x + image.shape[0]
    x2 = offset_x
    x3 = offset_x + image.shape[0]

    y0 = offset_y
    y1 = offset_y
    y2 = offset_y + image.shape[1]
    y3 = offset_y + image.shape[1]

    corners = np.zeros((3,4))
    corners[0,0] = x0
    corners[0,1] = x1
    corners[0,2] = x2
    corners[0,3] = x3
    corners[1,0] = y0
    corners[1,1] = y1
    corners[1,2] = y2
    corners[1,3] = y3
    corners[2:] = 1

    c = np.dot(R, corners)

    x = int(c[0,0])
    y = int(c[1,0])
    left = x
    right = x
    up = y
    down = y

    for i in range(4):
        x = int(c[0,i])
        y = int(c[1,i])
        if (x < left): left = x
        if (x > right): right = x
        if (y < up): up = y
        if (y > down): down = y
    h = down - up
    w = right - left

    cropped = np.zeros((w, h, 3), dtype='uint8')
    #rip his code, put an if here to account for the error
    if (up >= 0 and left >= 0):
        cropped[:, :, :] = dst_image[left:(left+w), up:(up+h), :]
    elif (up < 0 and left >= 0):
        cropped[:,:,:] = dst_image[left:(left + w), 0:h, :]
    elif (left < 0 and up >= 0):
        cropped[:,:,:] = dst_image[0:w, up:(up + h), :]
    else:
        cropped[:,:,:] = dst_image[0:w,0:h,:]
    return cropped

def getRotationsPlusMinus(angle, image):
    rotated1 = rotate_image(image, angle - 15)
    rotated1 = cv2.resize(rotated1, (40, 40), interpolation = cv2.INTER_CUBIC)

    rotated2 = rotate_image(image, angle)
    rotated2 = cv2.resize(rotated2, (40, 40), interpolation = cv2.INTER_CUBIC)

    rotated3 = rotate_image(image, angle + 15)
    rotated3 = cv2.resize(rotated3, (40, 40), interpolation = cv2.INTER_CUBIC)

    return (rotated1, rotated2, rotated3)

