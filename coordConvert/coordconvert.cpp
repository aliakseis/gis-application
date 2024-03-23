#include "coordconvert.h"

#include <cmath>


#define ALMOST_ZERO 0.0000001


#define LATITUDEAMEND_ENABLE
#define EARTH_ECCENT1 0.006694379990130 /* Eccentricity of earth elipsoid 1 */
#define EARTH_RADIUS 6378135.0          /* Radius of earth at equator [m] */
#define EARTH_ECCENT2 0.0067394         /* Eccentricity of earth elipsoid 2 */


static double _ROC_Longitude_;  // Geocentric longitude [degrees]
static double _ROC_Latitude_;   // Geocentric latitude [degrees]
static double _ROC_Altitude_;   // Geocentric altitude [meters]

static double _ROC_GEOCENTRIC_MATRIX_[3][3];  // Geocentric rotation matrix
static double _ROC_GEOCENTRIC_POLAR_[3];      // Geocentric polar coordinates

// Initialize geocentric coordinates and matrices here
/***********************************************************************
 *  Function Name:	Init_AxisTransformation
 *
 *  Description:	Initialize internal data. Get ROC location from
 *					configuration file/default value, and call init ROC
                                        location and init gnomonic tangency-point routines.
 *
 *  Specical Notes:	Must be called at init time.
 *
 *  Arguments:			name	in/out	type	description
 *					---------------------------------------------
 *					Longitude	input	double	ROC Longitude[degree]
 *					Latitude	input	double	ROC Latitude
 *
 *  Return Value:	None.
 ***********************************************************************/
void Init_CoordinateTransformation(double Longitude, /* ROC Longitude */
                                   double Latitude,  /* ROC Latitude */
                                   double Altitude)  /* ROC Altitude */
{
    _ROC_Longitude_ = Longitude; /* * ONEPI/180.0;*/
    _ROC_Latitude_ = Latitude;   /* * ONEPI/180.0;*/
    _ROC_Altitude_ = Altitude;

    Coordinate_Transform_Init_ROC_Location(_ROC_Longitude_, _ROC_Latitude_, _ROC_Altitude_);
}

/************************************************************************
 *	Function Name:	Coordinate_Transform_Init_ROC_Location
 *
 *  Description:	Given ROC location, precompute _ROC_GEOCENTRIC_MATRIX_
 *					and _ROC_GEOCENTRIC_POLAR_, and
 *					save in some static variables.
 *
 *  Arguments:			name	in/out	type	description
 *					---------------------------------------------
 *					Longitude	input	double	ROC Longitude
 *					Latitude	input	double	ROC Latitude
 *
 *  Return Value:	None.
 ********************************************************************/
void Coordinate_Transform_Init_ROC_Location(double Longitude, /* ROC Longitude */
                                            double Latitude,  /* ROC Latitude  */
                                            double Altitude)  /* ROC Altitude */
{
    /* 'ROC_Lat_Geocent' is ROC lat of geocentric */
    double ROC_Lat_Geocent = atan(tan(Latitude) / (1 + EARTH_ECCENT2));
    double sin_Long = sin(Longitude);
    double cos_Long = cos(Longitude);
    double sin_Lat = sin(ROC_Lat_Geocent);
    double cos_Lat = cos(ROC_Lat_Geocent);
    double Earth_Radius_ROC = EARTH_RADIUS / sqrt(1 + EARTH_ECCENT2 * sin_Lat * sin_Lat) + Altitude;

    /* the matrix to transport from earth coordinates to geocentric coordinates*/
#ifdef SYSTEM_SAEW
    _ROC_GEOCENTRIC_MATRIX_[0][0] = -sin_Lat * cos_Long;
    _ROC_GEOCENTRIC_MATRIX_[0][1] = sin_Long;
    _ROC_GEOCENTRIC_MATRIX_[0][2] = cos_Lat * cos_Long;

    _ROC_GEOCENTRIC_MATRIX_[1][0] = -sin_Lat * sin_Long;
    _ROC_GEOCENTRIC_MATRIX_[1][1] = -cos_Long;
    _ROC_GEOCENTRIC_MATRIX_[1][2] = cos_Lat * sin_Long;

    _ROC_GEOCENTRIC_MATRIX_[2][0] = cos_Lat;
    _ROC_GEOCENTRIC_MATRIX_[2][1] = 0;
    _ROC_GEOCENTRIC_MATRIX_[2][2] = sin_Lat;
#else
    _ROC_GEOCENTRIC_MATRIX_[0][0] = -sin_Long;
    _ROC_GEOCENTRIC_MATRIX_[0][1] = -sin_Lat * cos_Long;
    _ROC_GEOCENTRIC_MATRIX_[0][2] = cos_Lat * cos_Long;

    _ROC_GEOCENTRIC_MATRIX_[1][0] = cos_Long;
    _ROC_GEOCENTRIC_MATRIX_[1][1] = -sin_Lat * sin_Long;
    _ROC_GEOCENTRIC_MATRIX_[1][2] = cos_Lat * sin_Long;

    _ROC_GEOCENTRIC_MATRIX_[2][0] = 0;
    _ROC_GEOCENTRIC_MATRIX_[2][1] = cos_Lat;
    _ROC_GEOCENTRIC_MATRIX_[2][2] = sin_Lat;
#endif

    /* the vector - Ratios from coa to center of earth */
    _ROC_GEOCENTRIC_POLAR_[0] = Earth_Radius_ROC * _ROC_GEOCENTRIC_MATRIX_[0][2];
    _ROC_GEOCENTRIC_POLAR_[1] = Earth_Radius_ROC * _ROC_GEOCENTRIC_MATRIX_[1][2];
    _ROC_GEOCENTRIC_POLAR_[2] = Earth_Radius_ROC * _ROC_GEOCENTRIC_MATRIX_[2][2];
}

/***********************************************************************
 *  Function Name:	lax_tran_inrt_to_lglt
 *
 *  Description:	Transform Inertial to Longitude/Latitude.
 *
 *  Arguments:
 *		name	in/out	type					description
 *	-------------------------------------------------------------------
 *	Pos_ROCInner	input	INERTIAL_POSITION *	Position in Inertial coordinate system
 *	longitude	output	double				Longitude  [rad]
 *	latitude	output	double				Latitude   [rad]
 *	altitude	output	double				Altitude (Height above sea-level)[m]
 *
 *  Return Value:	None.
 **********************************************************************/
void Coordinate_Transform_ROCInner_To_LongLat(
    INERTIAL_POSITION *Pos_ROCInner, /* Position in Inertial coord. sys. */
     double *Longitude,              /* Longitude  [rad]  */
     double *Latitude,               /* Latitude   [rad]  */
     double *Altitude,               /* Height above sea-level  [m]  */
     double *Radaius)                /* Radius  [m]  */
{
    double x_gcs, y_gcs, z_gcs;
    double r_xyz, Earth_Radius;
    double r2_xy, r2_xyz;
    double sin2_Lat;

#ifdef LATITUDEAMEND_ENABLE
    double Kesai, Coefficient, Temp, Angle0, Angle;
#else
    double r_xy;
#endif

    x_gcs = _ROC_GEOCENTRIC_MATRIX_[0][0] * Pos_ROCInner->Inertial_X_f +
            _ROC_GEOCENTRIC_MATRIX_[0][1] * Pos_ROCInner->Inertial_Y_f +
            _ROC_GEOCENTRIC_MATRIX_[0][2] * Pos_ROCInner->Inertial_Z_f + _ROC_GEOCENTRIC_POLAR_[0];

    y_gcs = _ROC_GEOCENTRIC_MATRIX_[1][0] * Pos_ROCInner->Inertial_X_f +
            _ROC_GEOCENTRIC_MATRIX_[1][1] * Pos_ROCInner->Inertial_Y_f +
            _ROC_GEOCENTRIC_MATRIX_[1][2] * Pos_ROCInner->Inertial_Z_f + _ROC_GEOCENTRIC_POLAR_[1];

    z_gcs = _ROC_GEOCENTRIC_MATRIX_[2][0] * Pos_ROCInner->Inertial_X_f +
            _ROC_GEOCENTRIC_MATRIX_[2][1] * Pos_ROCInner->Inertial_Y_f +
            _ROC_GEOCENTRIC_MATRIX_[2][2] * Pos_ROCInner->Inertial_Z_f + _ROC_GEOCENTRIC_POLAR_[2];

    r2_xy = x_gcs * x_gcs + y_gcs * y_gcs;
    r2_xyz = r2_xy + z_gcs * z_gcs;
#ifndef LATITUDEAMEND_ENABLE
    r_xy = sqrt(r2_xy);
#endif
    r_xyz = sqrt(r2_xyz);

    sin2_Lat = z_gcs * z_gcs / r2_xyz;
    Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_ECCENT2 * sin2_Lat);

#ifdef LATITUDEAMEND_ENABLE
    Kesai = asin(sqrt(sin2_Lat));

    Coefficient = 0.5 * EARTH_ECCENT1 * EARTH_RADIUS / r_xyz;

    Angle0 = asin((Coefficient * sin(2 * Kesai)) / (sqrt(1 - EARTH_ECCENT1 * sin2_Lat)));
    Temp = 1 - 2 * Coefficient * cos(2 * Kesai) + 2 * Coefficient * Coefficient * sin2_Lat;
    if (fabs(Temp) > ALMOST_ZERO)
        Angle = Angle0 / Temp;
    else
        Angle = Angle0 / ALMOST_ZERO;

    *Latitude = Kesai + Angle;
#else
    *Latitude = atan((1 + EARTH_ECCENT2) * (z_gcs / r_xy));
#endif

    *Longitude = atan2(y_gcs, x_gcs);
    *Altitude = r_xyz - Earth_Radius;
    *Radaius = Earth_Radius;
}

/**********************************************************************
 *  Function Name:	Coordinate_Transform_LongLat_To_ROCInner
 *
 *  Description:	Transform Longitude/Latitude to Inertial.
 *
 *  Arguments:
 *		name	in/out	type					description
 *	-------------------------------------------------------------------
 *	longitude	input	double				Longitude  [rad]
 *	latitude	input	double				Latitude   [rad]
 *	altitude	input	double				Altitude (Height above sea-level)[m]
 *	Pos_ROCInner	output	INERTIAL_POSITION *	Position in Inertial coordinate system
 *
 *  Return Value:	None.
 **********************************************************************/
void Coordinate_Transform_LongLat_To_ROCInner(
    double Longitude,                 /* Longitude  [rad]  */
    double Latitude,                  /* Latitude   [rad]  */
    double Altitude,                  /* Height above sea-level  [m]  */
     INERTIAL_POSITION *Pos_ROCInner) /* Position in Inertial coord. sys. */
{
    double Lat_Geocent = atan(tan(Latitude) / (1 + EARTH_ECCENT2));
    double sin_Long = sin(Longitude);
    double cos_Long = cos(Longitude);
    double sin_Lat = sin(Lat_Geocent);
    double cos_Lat = cos(Lat_Geocent);
    double Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_ECCENT2 * sin_Lat * sin_Lat) + Altitude;

    /* x , y , z - geocentric */
    double x_gcs = Earth_Radius * cos_Lat * cos_Long;
    double y_gcs = Earth_Radius * cos_Lat * sin_Long;
    double z_gcs = Earth_Radius * sin_Lat;

    Pos_ROCInner->Inertial_X_f =
        _ROC_GEOCENTRIC_MATRIX_[0][0] * (x_gcs - _ROC_GEOCENTRIC_POLAR_[0]) +
        _ROC_GEOCENTRIC_MATRIX_[1][0] * (y_gcs - _ROC_GEOCENTRIC_POLAR_[1]) +
        _ROC_GEOCENTRIC_MATRIX_[2][0] * (z_gcs - _ROC_GEOCENTRIC_POLAR_[2]);

    Pos_ROCInner->Inertial_Y_f =
        _ROC_GEOCENTRIC_MATRIX_[0][1] * (x_gcs - _ROC_GEOCENTRIC_POLAR_[0]) +
        _ROC_GEOCENTRIC_MATRIX_[1][1] * (y_gcs - _ROC_GEOCENTRIC_POLAR_[1]) +
        _ROC_GEOCENTRIC_MATRIX_[2][1] * (z_gcs - _ROC_GEOCENTRIC_POLAR_[2]);

    Pos_ROCInner->Inertial_Z_f =
        _ROC_GEOCENTRIC_MATRIX_[0][2] * (x_gcs - _ROC_GEOCENTRIC_POLAR_[0]) +
        _ROC_GEOCENTRIC_MATRIX_[1][2] * (y_gcs - _ROC_GEOCENTRIC_POLAR_[1]) +
        _ROC_GEOCENTRIC_MATRIX_[2][2] * (z_gcs - _ROC_GEOCENTRIC_POLAR_[2]);
}
