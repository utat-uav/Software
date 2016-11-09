#include "Geolocation.h"
#include <math.h>
#include <iostream>

#ifndef PI
#define PI 3.14159265358979323846
#endif

/**
 * @brief Simple Constructor.
 *
 * @param[in] altitude Altitude of plane (in any unit).
 * @param[in] FOV_x Field of view along x-axis of camera (degrees).
 * @param[in] FOV_y Field of view along y-axis of camera (degrees).
 * @param[in] width Width of camera (pixels).
 * @param[in] height Height of camera (pixels).
 * @param[in] roll Roll angle of plane (degrees) (rotation around y-axis of camera).
 * @param[in] pitch Pitch angle of plane (degrees) (rotation around x-axis of camera).
 * @param[in] yaw Yaw angle of plane (degrees) (rotation around z-axis of camera)
 */
Geolocation::Geolocation(double altitude, double FOV_x, double FOV_y, int width, int height, double roll, double pitch, double yaw) {
    if (roll > 45) {
        std::cout << "Warning: Don't bother with roll > 45 degrees" << std::endl;
        roll = 45;
    }
    else if (roll < -45) {
        std::cout << "Warning: Don't bother with roll < -45 degrees" << std::endl;
        roll = -45;
    }
    if (pitch > 45) {
        std::cout << "Warning: Don't bother with pitch > 45 degrees" << std::endl;
        pitch = 45;
    }
    else if (pitch < -45) {
        pitch = -45;
        std::cout << "Warning: Don't bother with pitch < -45 degrees" << std::endl;
    }
    if (FOV_x > 60) {
        std::cout << "Warning: Don't bother with FOV_x > 60 degrees" << std::endl;
        FOV_x = 60;
    }
    else if (FOV_x < 1) {
        std::cout << "Warning: FOV_x = " << FOV_x << " which makes no sense" << std::endl;
        FOV_x = 1;
    }
    if (FOV_y > 60) {
        std::cout << "Warning: Don't bother with FOV_y > 60 degrees" << std::endl;
        FOV_y = 60;
    }
    else if (FOV_y < 1) {
        std::cout << "Warning: FOV_y = " << FOV_y << " which makes no sense" << std::endl;
        FOV_y = 1;
    }
    if (width < 1) {
        std::cout << "Warning: width " << width << " makes no sense" << std::endl;
        width = 1920;
    }
    if (height < 1) {
        std::cout << "Warning: height " << height << " makes no sense" << std::endl;
        height = 1080;
    }
    if (altitude <= 0) {
        std::cout << "Warning: altitude " << altitude << " makes no sense" << std::endl;
        altitude = 1;
    }

    this->altitude = altitude;
    this->FOV_x = FOV_x * PI / 180.;
    this->FOV_y = FOV_y * PI / 180.;
    this->width = width;
    this->height = height;
    this->roll = roll * PI / 180.;
    this->pitch = pitch * PI / 180.;
    this->yaw = yaw * PI / 180.;
}

/**
 * @brief Empty Destructor.
 */
Geolocation::~Geolocation(){
    // nothing to destruct
}

/**
 * @brief Calculate the distance to a point on the ground.
 *
 * Calculates the distance between the camera and a location on the ground, 
 * based on its pixel location in the image.
 *
 * @param[in] x The X coordinate of the pixel (min = 0, max = maxX).
 * @param[in] y The Y coordinate of the pixel (min = 0, max = maxX).
 * @param[out] east The distance east of the camera location (same units as altitude).
 * @param[out] north The distance north of the camera location (same units as altitude).
 */
void Geolocation::calcDistance(int x, int y, double& east, double& north) {

    // define a unit vectors for X Y and Z
    double ux[4] = {1., 0., 0., 1.};
    double uy[4] = {0., 1., 0., 1.};
    double uz[4] = {0., 0., 1., 1.};

    // rotate the coordinate axis for yaw
    rotateAbout(uz, yaw, ux);
    rotateAbout(uz, yaw, uy);

    // rotate the coordinate axis for pitch
    rotateAbout(ux, 0-pitch, uy);
    rotateAbout(ux, 0-pitch, uz);

    // rotate the coordinate axis for roll
    rotateAbout(uy, roll, ux);
    rotateAbout(uy, roll, uz);

    // check lengths of unit vectors

    // create a vector pointing in the negative x coordinate axis
    double u[3] = {0-uz[0], 0-uz[1], 0-uz[2]};

    // add x-axis to said vector to account for x position within the picture
    double factor = findXoverZ(x, width, FOV_x, 0);
    u[0] += factor * ux[0];
    u[1] += factor * ux[1];
    u[2] += factor * ux[2];
    factor = findXoverZ(y, height, FOV_y, 0);
    u[0] += factor * uy[0];
    u[1] += factor * uy[1];
    u[2] += factor * uy[2];

    double multiple = 0-altitude/u[2];
    east = multiple * u[0];
    north = multiple * u[1];
}

/**
 * @brief Calculate the change in X per unit Z.
 *
 * This function can be reused for Y, if you use the right FOV and the pitch 
 * instead of roll for tilt.
 *
 * @param[in] x Pixel location of X.
 * @param[in] maxX Image width.
 * @param[in] FOV Field of view along x.
 * @param[in] tilt Tilt along x-axis.
 */
double Geolocation::findXoverZ(int x, int width, double FOV, double tilt) {
    
    // Given the FOV, the furthest ray and middle ray (and by approximation, 
    // all the rays) converge at distance d behind the lens, where d is
    // measured in pixels.
    double d = width / 2 / tan(FOV);

    // If we put an imaginary stick of length d behind the lens at the
    // midpoint, the line passing through pixel x makes an angle alpha with
    // the imaginary line d.
    double alpha = atan(((double)x-(double)width/2.) / d);

    // Since the plane is tilted at angle tilt, the ray passing through the tip
    // of d and the pixel x makes angle theta with the normal to the ground (Z).
    double theta = alpha + tilt;

    // tan(theta) = deltaX / 1;
    return tan(theta);
}

/**
 * @brief Rotates a vector along the x-axis.
 */
void Geolocation::rotateX(double u[3], double theta) {
    double sine = sin(theta);
    double cosine = cos(theta);
    // u[0] untouched
    u[1] = u[1] * cosine + u[2] * sine;
    u[2] = 0-u[1] * sine + u[2] * cosine;
}


/**
 * @brief Rotates a vector along the y-axis.
 */
void Geolocation::rotateY(double u[3], double theta) {
    double sine = sin(theta);
    double cosine = cos(theta);
    u[0] = u[0] * cosine - u[2] * sine;
    // u[1] untouched
    u[2] = u[2] * cosine + u[0] * sine;
}

/**
 * @brief Rotates a vector along the z-axis.
 */
void Geolocation::rotateZ(double u[3], double theta) {
    double sine = sin(theta);
    double cosine = cos(theta);
    u[0] = u[0] * cosine - u[1] * sine;
    u[1] = 0-u[0] * sine + u[1] * cosine;
    // u[2] untouched
}

/**
 * @brief Rotates a vector along any axis.
 */
void Geolocation::rotateAbout(const double axis[4], double theta, double inout[4]) {
    // set up rotation matrix
    double rotationMatrix[4][4];
    double u = axis[0], v = axis[1], w = axis[2];
    double u2 = u * u, v2 = v * v, w2 = w * w;
    double L = (u2 + v2 + w2);
    double l = sqrt(L);
    double cosine = cos(theta), sine = sin(theta);

    rotationMatrix[0][0] = (u2 + (v2 + w2) * cosine) / L;
    rotationMatrix[0][1] = (u * v * (1 - cosine) - w * l * sine) / L;
    rotationMatrix[0][2] = (u * w * (1 - cosine) + v * l * sine) / L;
    rotationMatrix[0][3] = 0.0; 
 
    rotationMatrix[1][0] = (u * v * (1 - cosine) + w * l * sine) / L;
    rotationMatrix[1][1] = (v2 + (u2 + w2) * cosine) / L;
    rotationMatrix[1][2] = (v * w * (1 - cosine) - u * l * sine) / L;
    rotationMatrix[1][3] = 0.0; 
 
    rotationMatrix[2][0] = (u * w * (1 - cosine) - v * l * sine) / L;
    rotationMatrix[2][1] = (v * w * (1 - cosine) + u * l * sine) / L;
    rotationMatrix[2][2] = (w2 + (u2 + v2) * cosine) / L;
    rotationMatrix[2][3] = 0.0; 
 
    rotationMatrix[3][0] = 0.0;
    rotationMatrix[3][1] = 0.0;
    rotationMatrix[3][2] = 0.0;
    rotationMatrix[3][3] = 1.0;

    // make output and multiply
    double output[4];
    for (int c=0; c<4; ++c) {
        output[c] = 0;
        for (int k=0; k<4; k++) {
            output[c] += rotationMatrix[c][k] * inout[k];
        }
    }

    // copy into inout
    for (int c=0; c<4; ++c) {
        inout[c] = output[c];
    }
}

/**
 * @brief Finds the angle at which the pixel is relative to the focal point.
 */
double Geolocation::findAngle(int x, int width, double FOV) {
    
    // Given the FOV, the furthest ray and middle ray (and by approximation, 
    // all the rays) converge at distance d behind the lens, where d is
    // measured in pixels.
    double d = width / 2 / tan(FOV);

    // If we put an imaginary stick of length d behind the lens at the
    // midpoint, the line passing through pixel x makes an angle alpha with
    // the imaginary line d.
    return atan(((double)x-(double)width/2) / d);
}

/**
 * @brief Prints a vector.
 */
void Geolocation::printVector(const double u[3]) {
    std::cout << "X: " << u[0] << " Y: " << u[1] << " Z: " << u[2] << std::endl;
}
