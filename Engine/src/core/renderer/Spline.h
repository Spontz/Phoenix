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
		ChanVec	cv;
		float	tens, cont, bias;
		bool	linear;	// Linear interpolation
		float	step;

		KeyFrame() {
			for (int i = 0; i < kszKeyFrameNumChannels; i++)
				cv[i] = 0;
			tens = 0;
			cont = 0;
			bias = 0;
			linear = false;
			step = 0;
		}
	};

	// ******************************************************************

	// A spline is just an array of `keys' KeyFrames and the total number of
	// steps in the motion
	class Spline final {
	public:
		~Spline();

		std::vector<KeyFrame*>	key;		// Keyframes
		int32_t					keys;		// Number of Keyframes
		float					steps;		// Total steps of the spline
		int32_t					channels;	// Values of a keyframe

		std::string				filename;	// Filename
		float					duration;	// Spline duration in seconds

		Spline();

		static void Hermite(float t, float* h1, float* h2, float* h3, float* h4);
		void MotionCalcStep(ChanVec resVec, float step, bool loop);
		bool load();
	};
}