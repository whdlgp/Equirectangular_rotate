# Equirectangular_rotate
Rotate Equirectangular image with Eular angle

## Defendancy

* OpenCV

Currently tested with OpenCV 3.4.2

## How to use
```
Usage : Equirectangular_rotate.out <Image file name> <roll> <pitch> <yaw>
<roll>, <pitch>, <yaw> is rotation angle, It should be 0~360
```

## How to build (with Cmake)
```
git clone https://github.com/whdlgp/Equirectangular_rotate
cd Equirectangular_rotate
mkdir build && cd build
cmake ..
cmake --build .
```

## Example

Test with test image, Let's rotate image with Roll, Pitch, Yaw rotation

### Test image
<img src=https://github.com/whdlgp/Equirectangular_rotate/blob/master/test_image.JPG width="500" height="250">

### Rotate with Yaw angle, 30 degree
#### Command
```
./Equirectangular_rotate.out test_image.JPG 0 0 30 
```
#### Result image
<img src=https://github.com/whdlgp/Equirectangular_rotate/blob/master/rotate_0_0_30_test_image.JPG width="500" height="250">

### Rotate with Yaw and Pitch angle, 30 degree 
#### Command
```
./Equirectangular_rotate.out test_image.JPG 0 30 30 
```
#### Result image
<img src=https://github.com/whdlgp/Equirectangular_rotate/blob/master/rotate_0_30_30_test_image.JPG width="500" height="250">

### Rotate with Yaw, Pitch and Roll angle, 30 degree 
#### Command
```
./Equirectangular_rotate.out test_image.JPG 30 30 30 
```
#### Result image
<img src=https://github.com/whdlgp/Equirectangular_rotate/blob/master/rotate_30_30_30_test_image.JPG width="500" height="250">

