import cv2
import numpy as np
import sys
from math import pi, cos, sin, acos, atan2, sqrt
from tqdm import tqdm


def eular2rot(theta):
    R_x = np.array([[1, 0, 0],
                    [0, cos(theta[0]), -sin(theta[0])],
                    [0, sin(theta[0]), cos(theta[0])]])

    R_y = np.array([[cos(theta[1]), 0, sin(theta[1])],
                    [0, 1, 0],
                    [-sin(theta[1]), 0, cos(theta[1])]])

    R_z = np.array([[cos(theta[2]), -sin(theta[2]), 0],
                    [sin(theta[2]), cos(theta[2]), 0],
                    [0, 0, 1]])

    R = np.dot(R_x, np.dot(R_y, R_z))
    return R

def rot2eular(R):
    sy = sqrt(R[2, 2] * R[2, 2] + R[1, 2] * R[1, 2])

    singular = sy < 1e-6
    if not singular:
        x = atan2(-R[1, 2], R[2, 2])
        y = atan2(R[0, 2], sy)
        z = atan2(-R[0, 1], R[0, 0])
    else:
        x = 0
        y = atan2(R[0, 2], sy)
        z = atan2(-R[0, 1], R[0, 0])

    return np.array([x, y, z])

def rotate_pixel(in_vec, rot_mat, width, height):
    vec_rad = np.array([pi * in_vec[0] / height, 2 * pi * in_vec[1] / width])

    vec_cartesian = np.array([-sin(vec_rad[0]) * cos(vec_rad[1]),
                              sin(vec_rad[0]) * sin(vec_rad[1]),
                              cos(vec_rad[0])])

    vec_cartesian_rot = np.dot(rot_mat, vec_cartesian)

    vec_rot = np.array([acos(vec_cartesian_rot[2]), atan2(vec_cartesian_rot[1], -vec_cartesian_rot[0])])
    if vec_rot[1] < 0:
        vec_rot[1] += 2 * pi

    vec_pixel = np.array([int(height * vec_rot[0] / pi), int(width * vec_rot[1] / (2 * pi))])

    return vec_pixel

def main():
    if len(sys.argv) != 5:
        print("Usage : Equirectangular_rotate.py <Image file name> <roll> <pitch> <yaw>")
        print("<roll>, <pitch>, <yaw> is rotation angle, It should be 0~360")
        return

    im = cv2.imread(sys.argv[1])
    if im is None:
        print("Can't open image")
        return

    im_width = im.shape[1]
    im_height = im.shape[0]

    print("width: {}, height: {}".format(im_width, im_height))

    im_out = np.zeros_like(im)
    rot_mat = eular2rot([float(sys.argv[2]) * pi / 180.0, float(sys.argv[3]) * pi / 180.0, float(sys.argv[4]) * pi / 180.0])

    for i in tqdm(range(im_height)):
        for j in range(im_width):
            vec_pixel = rotate_pixel([i, j], rot_mat, im_width, im_height)
            origin_i, origin_j = vec_pixel

            if 0 <= origin_i < im_height and 0 <= origin_j < im_width:
                im_out[i, j] = im[origin_i, origin_j]

    savename = sys.argv[1]
    savename = "_" + savename
    savename = sys.argv[4] + savename
    savename = "_" + savename
    savename = sys.argv[3] + savename
    savename = "_" + savename
    savename = sys.argv[2] + savename
    savename = "rotate_" + savename

    print("Save to {}".format(savename))
    cv2.imwrite(savename, im_out)

if __name__ == "__main__":
    main()