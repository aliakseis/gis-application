#pragma once


struct INERTIAL_POSITION {
    double Inertial_X_f;
    double Inertial_Y_f;
    double Inertial_Z_f;
};


/* Temporary variables
 * Coordinate transformation and temporary variables for coordinate conversion, used for geocentric
 * coordinate transformation */
void Init_CoordinateTransformation(double Longitude, /* ROC Longitude */
                                    double Latitude,  /* ROC Latitude */
                                    double Altitude);

void Coordinate_Transform_Init_ROC_Location(double Longitude, /* ROC Longitude */
                                            double Latitude,  /* ROC Latitude */
                                            double Altitude);

void Coordinate_Transform_ROCInner_To_LongLat(
    INERTIAL_POSITION *Pos_ROCInner, /* Position in Inertial coord. sys. */
     double *Longitude,              /* Longitude  [rad]  */
     double *Latitude,               /* Latitude   [rad]  */
     double *Altitude,               /* Height above sea-level  [m]  */
     double *Radaius);               /* Height above sea-level  [m]  */

void Coordinate_Transform_LongLat_To_ROCInner(
    double Longitude,                  /* Longitude  [rad]  */
    double Latitude,                   /* Latitude   [rad]  */
    double Altitude,                   /* Height above sea-level  [m]  */
     INERTIAL_POSITION *Pos_ROCInner); /* Position in Inertial coord. sys. */
