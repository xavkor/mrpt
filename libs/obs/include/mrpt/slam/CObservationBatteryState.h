/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2014, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */
#ifndef CObservationBatteryState_H
#define CObservationBatteryState_H

#include <mrpt/utils/CSerializable.h>
#include <mrpt/slam/CObservation.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/poses/CPose2D.h>

namespace mrpt
{
namespace slam
{
	DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE_LINKAGE( CObservationBatteryState, CObservation, OBS_IMPEXP)

	/** This represents a measurement of the batteries on the robot.
	 *  The battery levels are in volts in the form of the public members:
	 *	- voltageMainRobotBattery
	 *	- voltageMainRobotComputer
	 *  - voltageOtherBatteries
	 *
	 *  There are boolean flags for signaling when the corresponding values have been filled out or not.
	 *
	 * \sa CObservation
	 * \ingroup mrpt_obs_grp
	 */
	class OBS_IMPEXP CObservationBatteryState : public CObservation
	{
		// This must be added to any CSerializable derived class:
		DEFINE_SERIALIZABLE( CObservationBatteryState )

	 public:
		/** Constructor
		 */
		CObservationBatteryState( );

		 /** The data members
		  * \sa voltageMainRobotBatteryIsValid,voltageMainRobotComputerIsValid
		  */
		double voltageMainRobotBattery, voltageMainRobotComputer;

		/** These values must be true if the corresponding fields contain valid values.
		  * \sa voltageMainRobotBattery,voltageMainRobotComputer
		  */
		bool   voltageMainRobotBatteryIsValid,voltageMainRobotComputerIsValid;

		/** The users can use this vector for any arbitrary number of batteries or any other analog measurements.
		  * \sa voltageOtherBatteriesValid
		  */
		vector_double voltageOtherBatteries;

		/** These values must be true if the corresponding fields contain valid values (it MUST has the same size than voltageOtherBatteries)
		  */
		vector_bool   voltageOtherBatteriesValid;

		/** A general method to retrieve the sensor pose on the robot.
		  *  It has no effects in this class
		  * \sa setSensorPose
		  */
		void getSensorPose( CPose3D &out_sensorPose ) const { out_sensorPose=CPose3D(0,0,0); }


		/** A general method to change the sensor pose on the robot.
		  *  It has no effects in this class
		  * \sa getSensorPose
		  */
		void setSensorPose( const CPose3D &newSensorPose ) {  }


	}; // End of class def.


	} // End of namespace
} // End of namespace

#endif
