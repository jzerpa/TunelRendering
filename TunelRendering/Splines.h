/*
  15-462 Computer Graphics I
  Assignment 2: Roller Coaster
  C++ Spline Utility Class
  Author: rtark
  Aug 2007

  NOTE: You do not need to edit this file for this assignment but may do so

  This file defines the following:
	Spline Base Class
	SplineList Class

  SplineList Usage: 3 primary functions
	SplineList::LoadSplines (trackFile) - load a track file
	SplineList::GetCurrent (points[4]) - get the current 4 control points
	SplineList::MoveToNext () - move forward by one control point
*/

#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "Utils.h"

/*
	Spline Class - A base spline class that contains the 4 control-points

	This is the underlying class used by the SplineList Class
*/
class Spline
{
	unsigned int length;
	Vector *points;
public:
	// -- Constructors & Destructors --
	Spline (void) : length (0), points (NULL) {}
	~Spline (void) { if (points) delete []points; }

	// -- Load Functions --
	// - File Loader - Loads the Spline from a file
	bool Load (char *splineFileName);
	// - Memory Loader - Loads the Spline from memory
	bool Load (Vector *pts, int numPoints);

	// -- Accessor Functions --
	// - GetLength - Returns the number of points in the spline
	unsigned int GetLength (void) { return length; }

	// - GetPoint - Returns the nth Spline point
	Vector GetPoint (int pointIndex);

};

/*
	SplineList Class - The main spline class that loads the roller coaster track

	This class loads the spline track to be accessed 4-points at a time
*/
class SplineList
{
	unsigned int numSplines;
	Spline *splines;
	int currentSpline;
	int currentPoint;
	Vector splinePoints[4];

public:
	// -- Constructors & Destructors --
	SplineList (void) : numSplines (0), splines (NULL), currentSpline (0), currentPoint (0), currentLength (0) {}
	~SplineList (void) { if (splines) delete []splines; }

	// -- Load Function --
	// - LoadSplines - Loads a track list
	bool LoadSplines (char *trackFileName);

	unsigned int currentLength;

	// -- Accessor Functions --
	// - GetSize - Returns the number of splines in the track
	unsigned int GetSize (void) { return numSplines; }

	// - GetSplineLength - Returns the length of the nth spline
	unsigned int GetSplineLength (int splineIndex);

	// - GetSplinePoint - Gets the point of nth spline
	Vector GetSplinePoint (int splineIndex, int pointIndex);

	// - GetCurrentSpline - Returns the index of the current Spline
	int GetCurrentSpline (void) { return currentSpline; }

	// - SetCurrentSpline - Sets the current Spline
	void SetCurrentSpline (int splineIndex) { if ((unsigned int)splineIndex < numSplines) {currentSpline = splineIndex; currentPoint = 0; } }

	// - GetCurrentPoint - Returns the index of the first of the 4 control-points of the spline sequence
	int GetCurrentPoint (void) { return currentPoint; }

	void SetCurrentPoint (int pointIndex) { if ((unsigned int)pointIndex < currentLength) { currentPoint = pointIndex; } }

	// -- Spline Iterator Functions --
	// - GetCurrent - Returns the current 4 control-points of the spline through a parameter
	void GetCurrent (Vector points[4]);

	// - MoveToNext - Moves forward by 1 control-point, moving to the next spline (loops back to the first spline at the end) as needed
	void MoveToNext (void);

	bool MoveToNextModif (void);
};



// --- SplineList Class Functions ---

bool SplineList::LoadSplines (char *trackFileName)
{
	FILE *trackFile = fopen (trackFileName, "r");
	char splineFilename[256];
	unsigned int n;

	if (trackFile == NULL)
	{
		return false;
	}

	// Read Number of Splines
	fscanf(trackFile, "%d", &numSplines);

	// Allocate Spline Array
	splines = new Spline[numSplines];
	// Not Enough Memory
	if (splines == NULL)
		return false;

	// Load the Spline Files
	for (n = 0; n < numSplines; n++)
	{
		int result = fscanf(trackFile, "%s", splineFilename);

		// Make sure the Spline Files load
		if (!splines[n].Load (splineFilename))
			return false;

		if (result == EOF)
			break;
	}

	// Check for premature EOF
	if (n < numSplines)
	{
		delete []splines;
		splines = NULL;

		return false;
	}

	// Load up the first Spline
	currentLength = splines[0].GetLength ();
	splinePoints[0] = splines[0].GetPoint (0);
	splinePoints[1] = splines[0].GetPoint (1);
	splinePoints[2] = splines[0].GetPoint (2);
	splinePoints[3] = splines[0].GetPoint (3);

	return true;
}

unsigned int SplineList::GetSplineLength (int splineIndex)
{
	if ((unsigned int)splineIndex < numSplines)
	{
		return splines[splineIndex].GetLength ();
	}

	return 0;
}

Vector SplineList::GetSplinePoint (int splineIndex, int pointIndex)
{
	if ((unsigned int)splineIndex < numSplines)
	{
		return splines[splineIndex].GetPoint (pointIndex);
	}

	return Vector ();
}

void SplineList::GetCurrent (Vector points[4])
{
	// Copy the points
	memcpy (points, splinePoints, sizeof(Vector) * 4);
}

void SplineList::MoveToNext (void)
{
	// Next point is 3-points past the (current one + 1)
	unsigned int nextPoint = currentPoint + 4, nextSpline = currentSpline;

	// Find the next Point from the new currentPoint
	if ((unsigned int)nextPoint >= currentLength)
	{
		nextSpline++;
		// Loop back to first spline
		if ((unsigned int)nextSpline >= numSplines)
			nextSpline = 0;

		nextPoint -= currentLength;
	}

	// Increment Current Point
	currentPoint++;

	// Move to the next Spline if necessary
	if ((unsigned int)currentPoint >= currentLength)
	{
		currentSpline++;
		// Loop back to first spline if necessary
		if ((unsigned int)currentSpline >= numSplines)
			currentSpline = 0;

		currentPoint = 0;
		currentLength = GetSplineLength (currentSpline);
	}

	// Push the Spline Points forward & load up the new point
	splinePoints[0] = splinePoints[1];
	splinePoints[1] = splinePoints[2];
	splinePoints[2] = splinePoints[3];
	splinePoints[3] = GetSplinePoint (nextSpline, nextPoint);
}

bool SplineList::MoveToNextModif (void)
{
	// Next point is 3-points past the (current one + 1)
	unsigned int nextPoint = currentPoint + 4, nextSpline = currentSpline;

	// Find the next Point from the new currentPoint
	/*if ((unsigned int)nextPoint >= currentLength)
	{
		nextSpline++;
		// Loop back to first spline
		if ((unsigned int)nextSpline >= numSplines)
			nextSpline = 0;

		nextPoint -= currentLength;
	}*/

	if ((unsigned int)nextPoint > currentLength-4)
	{
		return false;
	}

	// Increment Current Point
	currentPoint+=4;

	// Move to the next Spline if necessary
	/*if ((unsigned int)currentPoint >= currentLength)
	{
		currentSpline++;
		// Loop back to first spline if necessary
		if ((unsigned int)currentSpline >= numSplines)
			currentSpline = 0;

		currentPoint = 0;
		currentLength = GetSplineLength (currentSpline);
	}
	*/
	// Push the Spline Points forward & load up the new point
	splinePoints[0] = GetSplinePoint (nextSpline, nextPoint);
	splinePoints[1] = GetSplinePoint (nextSpline, nextPoint+1);
	splinePoints[2] = GetSplinePoint (nextSpline, nextPoint+2);
	splinePoints[3] = GetSplinePoint (nextSpline, nextPoint+3);
}


// --- Spline Class Functions ---

bool Spline::Load (char *splineFileName)
{
	int type; // dummy integer variable for spline file
	unsigned int n;

	// Open the spline file
	FILE *splineFile = fopen (splineFileName, "r");

	// Spline file failed
	if (splineFile == NULL)
	{
		return false;
    }

    // Read the length for spline file
    fscanf(splineFile, "%d %d", &length, &type);

	// Length must be >= 4 control points
	if (length < 4)
		return false;

    // Create the Array
    points = new Vector[length];
	// Not enough Memory
	if (points == NULL)
		return false;

    // Read the Spline data
	for (n = 0; n < length; n++)
	{
		if (fscanf(splineFile, "%f %f %f", &points[n].x, &points[n].y, &points[n].z) == EOF)
			break;
	}

	// Check for premature EOF
	if (n < length)
	{
		delete []points;
		points = NULL;

		return false;
	}

	return true;
}

bool Spline::Load (Vector *pts, int numPoints)
{
	if (pts == NULL)
		return false;

	length = numPoints;

	// Create the Array
	points = new Vector[length];
	// Not enough Memory
	if (points == NULL)
		return false;

	for (unsigned int n = 0; n < length; n++)
	{
		points[n] = pts[n];
	}

	return true;
}

Vector Spline::GetPoint (int pointIndex)
{
	if ((unsigned int)pointIndex < length)
	{
		return points[pointIndex];
	}

	return Vector ();
}

