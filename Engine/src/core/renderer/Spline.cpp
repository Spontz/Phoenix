// Spline.cpp
// Spontz Demogroup

#include <main.h>
#include <filesystem>
#include "core/renderer/Spline.h"

namespace Phoenix {

	Spline::Spline()
	{
		key.clear();
		keys = 0;
		steps = 0;
		channels = 0;
		filename = "";
		duration = 0;
	}

	Spline::~Spline()
	{
		for (auto const& pKeyFrame : key)
			delete pKeyFrame;
	}

	// Compute Hermite spline coeficients for t, where 0 <= t <= 1.
	void Spline::Hermite(float t, float* h1, float* h2, float* h3, float* h4)
	{
		float t2, t3, z;

		// h1(s) =  2s^3 - 3s^2 + 1
		// h2(s) = -2s^3 + 3s^2
		// h3(s) =   s^3 - 2s^2 + s
		// h4(s) =   s^3 -  s^2

		t2 = t * t;
		t3 = t * t2;
		z = 3.0f * t2 - t3 - t3;

		*h1 = 1.0f - z;
		*h2 = z;
		*h3 = t3 - t2 - t2 + t;
		*h4 = t3 - t2;
	}

	// Compute the motion channel vector for the given step. Step can be
	// fractional but values correspond to frames.
	void Spline::MotionCalcStep(ChanVec resVec, float step)
	{
		KeyFrame* key0, * key1;
		float t, h1, h2, h3, h4, res, d10;
		float dd0a = 0, dd0b = 0, ds1a = 0, ds1b = 0;
		float adj0 = 0, adj1 = 0, dd0 = 0, ds1 = 0;
		float tlength;
		int i;
		bool have_prev_key = true;	// Do we have a previous key, apart from key0?
		bool have_next_key = true;	// Do we have a next key, apart from key1?


		// If there is but one key, the values are constant.
		if (keys == 1) {
			for (i = 0; i < channels; i++)
				resVec[i] = key[0]->cv[i];
			return;
		}

		// Get keyframe pair to evaluate. This should be within the range
		// of the motion or this will raise an illegal access (fixed).
		int cnt = 0;
		for (cnt = 0; cnt < keys; cnt++) {
			if (key[cnt]->step >= step)
				break;
		}
		// Prevent invalid access when step is 0
		if (cnt != 0) {
			cnt--;
		}

		key0 = key[cnt];
		key1 = key[cnt + 1];
		// Check if we have previous and next keys
		if (cnt - 1 < 0)
			have_prev_key = false;
		if (cnt + 2 >= keys)
			have_next_key = false;

		step -= key0->step;

		// Get tween length and fractional tween position.
		tlength = key1->step - key0->step;
		t = step / tlength;

		// Precompute spline coefficients.
		if (key1->linear == false) {
			Hermite(t, &h1, &h2, &h3, &h4);
			dd0a = (1.0f - key0->tens) * (1.0f + key0->cont) * (1.0f + key0->bias);
			dd0b = (1.0f - key0->tens) * (1.0f - key0->cont) * (1.0f - key0->bias);
			ds1a = (1.0f - key1->tens) * (1.0f - key1->cont) * (1.0f + key1->bias);
			ds1b = (1.0f - key1->tens) * (1.0f + key1->cont) * (1.0f - key1->bias);

			// First we check if Key0 is not the step 0 or 1
			if (have_prev_key)
				adj0 = tlength / (key1->step - key[cnt - 1]->step);

			// First we check if its not the last step or last step+1
			if (have_next_key)
				adj1 = tlength / (key[cnt + 2]->step - key0->step);
		}

		// Compute the channel components.
		for (i = 0; i < channels; i++) {
			d10 = key1->cv[i] - key0->cv[i];

			if (key1->linear == false) {
				if (!have_prev_key)
					dd0 = 0.5f * (dd0a + dd0b) * d10;
				else
					dd0 = adj0 * (dd0a * (key0->cv[i] - key[cnt - 1]->cv[i]) + dd0b * d10);

				if (!have_next_key)
					ds1 = 0.5f * (ds1a + ds1b) * d10;
				else
					ds1 = adj1 * (ds1a * d10 + ds1b * (key[cnt + 2]->cv[i] - key1->cv[i]));

				res = key0->cv[i] * h1 + key1->cv[i] * h2 + dd0 * h3 + ds1 * h4;
			}
			else
				res = key0->cv[i] + t * d10;

			resVec[i] = res;
		}

	}

	bool Spline::load()
	{
		std::string fileData = Utils::readASCIIFile(filename);

		// Convert the fileData to StringStream so it can be processed line by line
		std::stringstream ssFileData;
		ssFileData.str(fileData);

		std::string line;
		uint32_t lineCnt = 0;

		key.clear();
		keys = 0;
		steps = 0;
		channels = 0;

		while (std::getline(ssFileData, line)) {
			// comments or empty line
			if ((line[0] == ';') || (line[0] == '\n') || (line[0] == '\r')) continue;

			// Get the line values, using the 'tab' separator
			std::stringstream lineStream(line);
			std::string value;
			std::vector<std::string> valueList;
			int32_t lineChannels = 0;

			while (std::getline(lineStream, value, '\t')) {
				valueList.push_back(value);
			}

			// Validate the line
			lineChannels = static_cast<int32_t> (valueList.size());

			if (lineChannels >= kszKeyFrameNumChannels) {
				Logger::error("Spline load error: too many floats in file: {}, line {}: Found {} values, but max is {}", filename, lineCnt, channels, kszKeyFrameNumChannels);
				return false;
			}
			if (lineChannels == 0) {
				Logger::error("Spline load error : incorrect format in {}, line {}: Zero values detected", filename, lineCnt);
				return false;
			}

			// Validate if the current line has the same number of channels of the first line
			if (key.size() == 0) {
				channels = lineChannels;
			}
			else {
				if (lineChannels != channels) {
					Logger::error("Spline load error : incorrect format in {}, line {} has {} values, but it should be {} as per first line", filename, lineCnt, lineChannels, channels);
					return false;
				}

			}

			// Load the values into the KeyFrame
			KeyFrame* newKey = new KeyFrame();
			for (int i = 0; i < channels; i++) {
				newKey->cv[i] = std::stof(valueList[i]);
			}

			key.push_back(newKey);
			//Logger::info(LogLevel::high, "Line read: {}", line.c_str());
			lineCnt++;
		}


		// Calculate the steps of each key based in the duration
		keys = static_cast<int32_t> (key.size());
		steps = duration;
		float motionStepTime = steps / (keys - 1);
		for (int i = 0; i < keys; i++)
			key[i]->step = motionStepTime * i;

		return true;
	}
}
