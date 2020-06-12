// spline.h
// Spontz Demogroup

#ifndef SPLINE_H
#define SPLINE_H
#include <vector>
#include <string>

// ******************************************************************

// LightWave: A motion tracks has 9 channels (x, y, z position, h, p, b rotation angle,
// and x, y, z scale), although this number can vary.  Envelopes work the
// same way but track only one channel.

#define NUM_CHAN 32
typedef float ChanVec[NUM_CHAN];

// Each KeyFrame in a motion has the value of the channels at that key,
// tension, continuity and bias spline controls, a linear flag and the
// frame at which the key is located (step).
typedef struct st_KeyFrame {
	ChanVec cv;
	float   tens, cont, bias;
	int     linear;
	float   step;
} KeyFrame;

// ******************************************************************

// A spline is just an array of `keys' KeyFrames and the total number of
// steps in the motion
class Spline {
public:
	std::vector<KeyFrame*>	key;
	int						keys;		// KeyNumber
	float					steps;		// total steps of the spline
	int						channels;	// Columns

	std::string				filename;	// Filename
	float					duration;	// Spline duration in seconds

	Spline();
	virtual ~Spline();
	
	static void Hermite(float t, float *h1, float *h2, float *h3, float *h4);
	void MotionCalcStep(ChanVec resVec, float step);
	bool load();

private:

};

#endif