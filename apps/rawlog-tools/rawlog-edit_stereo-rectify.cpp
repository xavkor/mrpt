/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                       http://www.mrpt.org/                                |
   |                                                                           |
   |   Copyright (C) 2005-2012  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include "rawlog-edit-declarations.h"
#include <mrpt/vision/CStereoRectifyMap.h>

using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::slam;
using namespace mrpt::system;
using namespace mrpt::rawlogtools;
using namespace std;


// ======================================================================
//		op_stereo_rectify
// ======================================================================
DECLARE_OP_FUNCTION(op_stereo_rectify)
{
	// A class to do this operation:
	class CRawlogProcessor_StereoRectify : public CRawlogProcessorOnEachObservation
	{
	protected:
		TOutputRawlogCreator	outrawlog;

		string   target_label;
		string   outDir;
		string   imgFileExtension;
		double   rectify_alpha; // [0,1] see cvStereoRectify()

		mrpt::vision::CStereoRectifyMap   rectify_map;

	public:
		size_t  m_changedCams;

		CRawlogProcessor_StereoRectify(CFileGZInputStream &in_rawlog, TCLAP::CmdLine &cmdline, bool verbose) :
			CRawlogProcessorOnEachObservation(in_rawlog,cmdline,verbose)
		{
			m_changedCams = 0;

			// Load .ini file with poses:
			string   str;
			getArgValue<string>(cmdline,"stereo-rectify",str);

			vector<string> lstTokens;
			tokenize(str,",",lstTokens);
			if (lstTokens.size()!=2)
				throw std::runtime_error("--stereo-rectify op: argument must be in the format: --stereo-rectify LABEL,ALPHA_VALUE");

			target_label = lstTokens[0];

			const string &sAlpha = lstTokens[1];
			rectify_alpha = atof(sAlpha.c_str());
			if (rectify_alpha!=-1 && !(rectify_alpha>=0 && rectify_alpha<=1))
				throw std::runtime_error("--stereo-rectify op: Invalid ALPHA value. Use '-1' for auto guess.");

			getArgValue<string>(cmdline,"image-format",imgFileExtension);

			// Create a "/Images_Rectified" directory.
			const string out_rawlog_basedir = extractFileDirectory(outrawlog.out_rawlog_filename);

			outDir = (out_rawlog_basedir.empty() ? string() : (out_rawlog_basedir+string("/") )) + extractFileName(outrawlog.out_rawlog_filename) + string("_Images");
			if (directoryExists(outDir))
				throw runtime_error(string("*ABORTING*: Output directory for rectified images already exists: ") + outDir + string("\n. Select a different output path or remove the directory.") );

			VERBOSE_COUT << "Creating directory: " << outDir << endl;

			mrpt::system::createDirectory( outDir );
			if (!fileExists(outDir))
				throw runtime_error(string("*ABORTING*: Couldn't create directory: ") + outDir );

			// Add the final /
			outDir+="/";

			// Optional argument:  "--image-size=640x480"
			string   strResize;
			if (getArgValue<string>(cmdline,"image-size",str))
			{
                vector<string> lstTokens;
                tokenize(str,"x",lstTokens);
                if (lstTokens.size()!=2)
                    throw std::runtime_error("--stereo-rectify op: Expected format: --image-size NCOLSxNROWS");

                const int nCols = atoi(lstTokens[0].c_str());
                const int nRows = atoi(lstTokens[1].c_str());
                VERBOSE_COUT << "Will rectify and resize to " << nCols << "x" << nRows << " simultaneously.\n";

                rectify_map.enableResizeOutput(true,nCols,nRows);
			}
		}

		bool processOneObservation(CObservationPtr  &obs)
		{
			if ( strCmpI(obs->sensorLabel,target_label))
			{
				if (IS_CLASS(obs,CObservationStereoImages))
				{
					CObservationStereoImagesPtr o = CObservationStereoImagesPtr(obs);

					// Already initialized the rectification map?
					if (!rectify_map.isSet())
					{
						// On the first ocassion, initialize map:
						rectify_map.setAlpha( rectify_alpha );
						rectify_map.setFromCamParams( *o );
					}

					// This call rectifies the images in-place and also updates
					// all the camera parameters as needed:
					rectify_map.rectify(*o);

					const string label_time = format("%s_%f", o->sensorLabel.c_str(), timestampTotime_t(o->timestamp) );
					{
						const string fileName = string("img_") + label_time + string("_left.") + imgFileExtension;
						o->imageLeft.saveToFile( outDir + fileName );
						o->imageLeft.setExternalStorage( fileName );
					}
					{
						const string fileName = string("img_") + label_time + string("_right.") + imgFileExtension;
						o->imageRight.saveToFile( outDir + fileName );
						o->imageRight.setExternalStorage( fileName );
					}
					m_changedCams++;
				}
			}
			return true;
		}

		// This method can be reimplemented to save the modified object to an output stream.
		virtual void OnPostProcess(
			mrpt::slam::CActionCollectionPtr &actions,
			mrpt::slam::CSensoryFramePtr     &SF,
			mrpt::slam::CObservationPtr      &obs)
		{
			ASSERT_((actions && SF) || obs)
			if (actions)
					outrawlog.out_rawlog << actions << SF;
			else	outrawlog.out_rawlog << obs;
		}

	};

	// Process
	// ---------------------------------
	CRawlogProcessor_StereoRectify proc(in_rawlog,cmdline,verbose);
	proc.doProcessRawlog();

	// Dump statistics:
	// ---------------------------------
	VERBOSE_COUT << "Time to process file (sec)        : " << proc.m_timToParse << "\n";
	VERBOSE_COUT << "Number of modified entries        : " << proc.m_changedCams << "\n";

}
