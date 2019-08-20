#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <omp.h>

#define RAD(x) M_PI*(x)/180.0
#define DEGREE(x) 180.0*(x)/M_PI

#include <opencv2/opencv.hpp>

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

// Wrapper of rodrigues rotation vector <-> matrix conversion
Mat eular2rot(Vec3d theta)
{
    Mat R;
    Rodrigues(theta, R);
    return R;
}

Vec3d rot2eular(Mat rot_mat)
{
    Mat vec_mat;
    Rodrigues(rot_mat, vec_mat);
    double* vec_mat_data = (double*)vec_mat.data;
    Vec3d rot_vec_inv_rodrig(vec_mat_data[0], vec_mat_data[1], vec_mat_data[2]);
    return rot_vec_inv_rodrig;
}

// rotate pixel, in_vec as input(row, col)
Vec2i rotate_pixel(const Vec2i& in_vec, Mat& rot_mat, int width, int height)
{
    Vec2d vec_rad = Vec2d(M_PI*in_vec[0]/height, 2*M_PI*in_vec[1]/width);

    Vec3d vec_cartesian;
    vec_cartesian[0] = sin(vec_rad[0])*cos(vec_rad[1]);
    vec_cartesian[1] = sin(vec_rad[0])*sin(vec_rad[1]);
    vec_cartesian[2] = cos(vec_rad[0]);

    double* rot_mat_data = (double*)rot_mat.data;
    Vec3d vec_cartesian_rot;
    vec_cartesian_rot[0] = rot_mat_data[0]*vec_cartesian[0] + rot_mat_data[1]*vec_cartesian[1] + rot_mat_data[2]*vec_cartesian[2];
    vec_cartesian_rot[1] = rot_mat_data[3]*vec_cartesian[0] + rot_mat_data[4]*vec_cartesian[1] + rot_mat_data[5]*vec_cartesian[2];
    vec_cartesian_rot[2] = rot_mat_data[6]*vec_cartesian[0] + rot_mat_data[7]*vec_cartesian[1] + rot_mat_data[8]*vec_cartesian[2];

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
    Mat im_out(im.rows, im.cols, im.type());
    Vec3b* im_data = (Vec3b*)im.data;
    Vec3b* im_out_data = (Vec3b*)im_out.data;
    //Mat rot_mat = eular2rot(Vec3f(-RAD(atof(argv[2])), -RAD(atof(argv[3])), -RAD(atof(argv[4]))));
    Mat rot_mat = eular2rot(Vec3f(RAD(atof(argv[2])), RAD(atof(argv[3])), RAD(atof(argv[4])))).t();
    #pragma omp parallel for
    for(int i = 0; i < static_cast<int>(im_height); i++)
    {
        for(int j = 0; j < static_cast<int>(im_width); j++)
        {
            // inverse warping
            Vec2i vec_pixel = rotate_pixel(Vec2i(i, j) 
                                         , rot_mat
                                         , im_width, im_height);
            int origin_i = vec_pixel[0];
            int origin_j = vec_pixel[1];
            if((origin_i >= 0) && (origin_j >= 0) && (origin_i < im_height) && (origin_j < im_width))
            {
                im_out_data[i*im.cols + j] = im_data[origin_i*im.cols + origin_j];
            }
        }
        if(omp_get_thread_num() == 0)
            draw_progress((i*1.0f/(im_height/omp_get_num_threads())));
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