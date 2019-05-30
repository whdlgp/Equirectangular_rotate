#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define RAD(x) M_PI*(x)/180.0
#define DEGREE(x) 180.0*(x)/M_PI

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

// From OpenCV example utils.hpp code
// Calculates rotation matrix given euler angles.
Mat eular2rot(Vec3f theta)
{
    // Calculate rotation about x axis
    Mat R_x = (Mat_<double>(3,3) <<
               1,       0,              0,
               0,       cos(theta[0]),   -sin(theta[0]),
               0,       sin(theta[0]),   cos(theta[0])
               );
     
    // Calculate rotation about y axis
    Mat R_y = (Mat_<double>(3,3) <<
               cos(theta[1]),    0,      sin(theta[1]),
               0,               1,      0,
               -sin(theta[1]),   0,      cos(theta[1])
               );
     
    // Calculate rotation about z axis
    Mat R_z = (Mat_<double>(3,3) <<
               cos(theta[2]),    -sin(theta[2]),      0,
               sin(theta[2]),    cos(theta[2]),       0,
               0,               0,                  1);
     
     
    // Combined rotation matrix
    Mat R = R_z * R_y * R_x;
     
    return R;
}

int main(int argc, char** argv)
{
    if(argc != 5)
    {
        cout << "Usage : Equirectangular_rotate.out <Image file name> <roll> <pitch> <yaw>" << endl;
        cout << "<roll>, <pitch>, <yaw> is rotation angle, It should be 0~360" << endl;
        return 0;
    }
    Mat im = imread(argv[1]);
    if(im.data == NULL)
    {
        cout << "Can't open image" << endl;
        return 0;
    }

    double im_width = im.cols;
    double im_height = im.rows;
    double im_size = im_width*im_height;
    Size im_shape(im_height, im_width);

    cout << "width : " << im_width << ", height : " << im_height << endl;

    // Rotation matrix for rotating image
    cout << "Rotation matrix for rotating image" << endl;
    Mat rot_mat = eular2rot(Vec3f(RAD(atof(argv[2])), RAD(atof(argv[3])), RAD(atof(argv[4]))));
    cout << rot_mat << endl;

    // For inverse mapping, need inverse matrix of Rotation matrix
    cout << "Inverse matrix of Rotation matrix" << endl;
    Mat rot_mat_inv = rot_mat.inv();
    cout << rot_mat_inv << endl;

    // (row, column) pixel coordinate to (lat, lon) spherical coordinate distance r is 1.0
    // For target image's coordinate
    cout << "Inverse warping, search target's pixel value from original image" << endl;
    cout << "(row, column) pixel coordinate to (lat, lon) spherical coordinate distance r is 1.0" << endl;
    Mat_<Vec2d> im_rad(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            im_rad.at<Vec2d>(i, j) = Vec2d(M_PI*i/im_height
                                         , 2*M_PI*j/im_width);
        }
    }

    // (lat, lon) spherical coordinate to (x, y, z) coordinate
    cout << "(lat, lon) spherical coordinate to (x, y, z) coordinate" << endl;
    Mat_<Vec3d> im_unit_sphere(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            Vec3d vec_cartesian;
            vec_cartesian[0] = sin(im_rad.at<Vec2d>(i, j)[0])*cos(im_rad.at<Vec2d>(i, j)[1]);
            vec_cartesian[1] = sin(im_rad.at<Vec2d>(i, j)[0])*sin(im_rad.at<Vec2d>(i, j)[1]);
            vec_cartesian[2] = cos(im_rad.at<Vec2d>(i, j)[0]);
            im_unit_sphere.at<Vec3d>(i, j) = vec_cartesian;
        }
    }

    // Apply rotation matrix
    cout << "Apply inverse rotation matrix" << endl;
    Mat_<Vec3d> im_unit_sphere_rotate(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            Vec3d vec_cartesian_rot;
            vec_cartesian_rot[0] = rot_mat_inv.at<double>(0, 0)*im_unit_sphere.at<Vec3d>(i, j)[0] + rot_mat_inv.at<double>(0, 1)*im_unit_sphere.at<Vec3d>(i, j)[1] + rot_mat_inv.at<double>(0, 2)*im_unit_sphere.at<Vec3d>(i, j)[2];
            vec_cartesian_rot[1] = rot_mat_inv.at<double>(1, 0)*im_unit_sphere.at<Vec3d>(i, j)[0] + rot_mat_inv.at<double>(1, 1)*im_unit_sphere.at<Vec3d>(i, j)[1] + rot_mat_inv.at<double>(1, 2)*im_unit_sphere.at<Vec3d>(i, j)[2];
            vec_cartesian_rot[2] = rot_mat_inv.at<double>(2, 0)*im_unit_sphere.at<Vec3d>(i, j)[0] + rot_mat_inv.at<double>(2, 1)*im_unit_sphere.at<Vec3d>(i, j)[1] + rot_mat_inv.at<double>(2, 2)*im_unit_sphere.at<Vec3d>(i, j)[2];
            im_unit_sphere_rotate.at<Vec3d>(i, j) = vec_cartesian_rot;
        }
    }

    // (x, y, z) coordinate to (lat, lon) spherical coordinate distance r is 1.0
    cout << "(x, y, z) coordinate to (lat, lon) spherical coordinate distance r is 1.0" << endl;
    Mat_<Vec2d> im_rad_rotate(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            Vec2d vec_rot;
            vec_rot[0] = acos(im_unit_sphere_rotate.at<Vec3d>(i, j)[2]);
            vec_rot[1] = atan2(im_unit_sphere_rotate.at<Vec3d>(i, j)[1], im_unit_sphere_rotate.at<Vec3d>(i, j)[0]);
            if(vec_rot[1] < 0)
                vec_rot[1] += M_PI*2;
            im_rad_rotate.at<Vec2d>(i, j) = vec_rot;
        }
    }

    // (lat, lon) spherical coordinate to (row, column) pixel coordinate
    cout << "(lat, lon) spherical coordinate to (row, column) pixel coordinate" << endl;
    Mat_<Vec2d> im_pixel_rotate(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            Vec2d vec_pixel;
            vec_pixel[0] = im_height*im_rad_rotate.at<Vec2d>(i, j)[0]/M_PI;
            vec_pixel[1] = im_width*im_rad_rotate.at<Vec2d>(i, j)[1]/(2*M_PI);
            im_pixel_rotate.at<Vec2d>(i, j) = vec_pixel;
        }
    }

    // save image
    cout << "Save image" << endl;
    Mat im_out(im.rows, im.cols, im.type());
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            int out_i = im_pixel_rotate.at<Vec2d>(i, j)[0];
            int out_j = im_pixel_rotate.at<Vec2d>(i, j)[1];
            if((out_i >= 0) && (out_j >= 0) && (out_i < im_height) && (out_j < im_width))
            {
                im_out.at<Vec3b>(i, j) = im.at<Vec3b>(out_i, out_j);
            }
        }
    }

    String savename = argv[1];
    savename = "_" + savename;
    savename = argv[4] + savename;
    savename = "_" + savename;
    savename = argv[3] + savename;
    savename = "_" + savename;
    savename = argv[2] + savename;
    savename = "rotate_" + savename;
    cout << "Save to " << savename << endl;
    imwrite(savename, im_out);

    return 0;
}