// Spline.h
// Spontz Demogroup

#pragma once

#include <vector>
#include <string>

namespace Phoenix {

	// ******************************************************************

	// LightWave: A motion tracks has 9 channels (x, y, z position, h, p, b rotation angle,
	// and x, y, z scale), although this number can vary.  Envelopes work the
	// same way but track only one channel.

	static constexpr size_t kszKeyFrameNumChannels = 32;

	using ChanVec = float[kszKeyFrameNumChannels];

	// Each KeyFrame in a motion has the value of the channels at that key,
	// tension, continuity and bias spline controls, a linear flag and the
	// frame at which the key is located (step).
	struct KeyFrame final {
		ChanVec cv;
		float   tens, cont, bias;
		int     linear;
		float   step;
	};

	// ******************************************************************

	// A spline is just an array of `keys' KeyFrames and the total number of
	// steps in the motion
	class Spline final {
	public:
		std::vector<KeyFrame*>	key;
		int						keys;		// KeyNumber
		float					steps;		// total steps of the spline
		int						channels;	// Columns

		std::string				filename;	// Filename
		float					duration;	// Spline duration in seconds

		Spline();
		~Spline();

		static void Hermite(float t, float* h1, float* h2, float* h3, float* h4);
		void MotionCalcStep(ChanVec resVec, float step);
		bool load();
	};
}