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

void draw_progress(float progress)
{
    std::cout << "[";
    int bar_width = 70;
    int pos = bar_width * progress;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

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

// rotate pixel, in_vec as input(row, col)
Vec2i rotate_pixel(const Vec2i& in_vec, Vec3f theta, int width, int height)
{
    Mat rot_mat = eular2rot(theta);

    Vec2d vec_rad = Vec2d(M_PI*in_vec[0]/height, 2*M_PI*in_vec[1]/width);

    Vec3d vec_cartesian;
    vec_cartesian[0] = sin(vec_rad[0])*cos(vec_rad[1]);
    vec_cartesian[1] = sin(vec_rad[0])*sin(vec_rad[1]);
    vec_cartesian[2] = cos(vec_rad[0]);

    Vec3d vec_cartesian_rot;
    vec_cartesian_rot[0] = rot_mat.at<double>(0, 0)*vec_cartesian[0] + rot_mat.at<double>(0, 1)*vec_cartesian[1] + rot_mat.at<double>(0, 2)*vec_cartesian[2];
    vec_cartesian_rot[1] = rot_mat.at<double>(1, 0)*vec_cartesian[0] + rot_mat.at<double>(1, 1)*vec_cartesian[1] + rot_mat.at<double>(1, 2)*vec_cartesian[2];
    vec_cartesian_rot[2] = rot_mat.at<double>(2, 0)*vec_cartesian[0] + rot_mat.at<double>(2, 1)*vec_cartesian[1] + rot_mat.at<double>(2, 2)*vec_cartesian[2];

    Vec2d vec_rot;
    vec_rot[0] = acos(vec_cartesian_rot[2]);
    vec_rot[1] = atan2(vec_cartesian_rot[1], vec_cartesian_rot[0]);
    if(vec_rot[1] < 0)
        vec_rot[1] += M_PI*2;

    Vec2i vec_pixel;
    vec_pixel[0] = height*vec_rot[0]/M_PI;
    vec_pixel[1] = width*vec_rot[1]/(2*M_PI);

    return vec_pixel;
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

    Mat2i im_pixel_rotate(im_height, im_width);
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            Vec2i vec_pixel = rotate_pixel(Vec2i(i, j) 
                                         , Vec3f(RAD(atof(argv[2]))
                                               , RAD(atof(argv[3]))
                                               , RAD(atof(argv[4])))
                                         , im_width, im_height);
            im_pixel_rotate.at<Vec2i>(i, j) = vec_pixel;
        }

        draw_progress((i*1.0f/im_height));
    }

    // save image
    cout << "Save image" << endl;
    Mat im_out(im.rows, im.cols, im.type());
    for(int i = 0; i < im_height; i++)
    {
        for(int j = 0; j < im_width; j++)
        {
            int out_i = im_pixel_rotate.at<Vec2i>(i, j)[0];
            int out_j = im_pixel_rotate.at<Vec2i>(i, j)[1];
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