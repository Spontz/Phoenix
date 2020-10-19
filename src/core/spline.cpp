// spline.cpp
// Spontz Demogroup

#include <main.h>
#include "spline.h"

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
	key.clear();
	keys = 0;
	steps = 0;
	channels = 0;
	filename = "";
	duration = 0;
}

// Compute Hermite spline coeficients for t, where 0 <= t <= 1.
void Spline::Hermite(float t, float * h1, float * h2, float * h3, float * h4)
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
	KeyFrame *key0, *key1;
	float t, h1, h2, h3, h4, res, d10;
	float dd0a = 0, dd0b = 0, ds1a = 0, ds1b = 0;
	float adj0 = 0, adj1 = 0, dd0 = 0, ds1 = 0;
	float tlength;
	int i;
	bool have_prev_key = true;	// Do we have a previous key, apart from key0?
	bool have_next_key = true;	// Do we have a next key, apart from key1?


	// If there is but one key, the values are constant.
	if (this->keys == 1) {
		for (i = 0; i < this->channels; i++)
			resVec[i] = this->key[0]->cv[i];
		return;
	}

	// Get keyframe pair to evaluate. This should be within the range
	// of the motion or this will raise an illegal access (fixed).
	int cnt=0;
	for (cnt = 0; cnt < this->keys; cnt++) {
		if (this->key[cnt]->step >= step)
			break;
	}
	// Prevent invalid access when step is 0
	if (cnt != 0) {
		cnt--;
	}

	key0 = this->key[cnt];
	key1 = this->key[cnt + 1];
	// Check if we have previous and next keys
	if (cnt - 1 < 0)
		have_prev_key = false;
	if (cnt + 2 >= this->keys)
		have_next_key = false;

	step -= key0->step;

	// Get tween length and fractional tween position.
	tlength = key1->step - key0->step;
	t = step / tlength;

	// Precompute spline coefficients.
	if (!key1->linear) {
		Hermite(t, &h1, &h2, &h3, &h4);
		dd0a = (1.0f - key0->tens) * (1.0f + key0->cont) * (1.0f + key0->bias);
		dd0b = (1.0f - key0->tens) * (1.0f - key0->cont) * (1.0f - key0->bias);
		ds1a = (1.0f - key1->tens) * (1.0f - key1->cont) * (1.0f + key1->bias);
		ds1b = (1.0f - key1->tens) * (1.0f + key1->cont) * (1.0f - key1->bias);

		// First we check if Key0 is not the step 0 or 1
		if (have_prev_key)
			adj0 = tlength / (key1->step - this->key[cnt - 1]->step);
		
		// First we check if its not the last step or last step+1
		if (have_next_key)
			adj1 = tlength / (this->key[cnt + 2]->step - key0->step);
	}

	// Compute the channel components.
	for (i = 0; i < this->channels; i++) {
		d10 = key1->cv[i] - key0->cv[i];

		if (!key1->linear) {
			if (!have_prev_key)
				dd0 = 0.5f * (dd0a + dd0b) * d10;
			else
				dd0 = adj0 * (dd0a * (key0->cv[i] - this->key[cnt-1]->cv[i]) + dd0b * d10);

			if (!have_next_key)
				ds1 = 0.5f * (ds1a + ds1b) * d10;
			else
				ds1 = adj1 * (ds1a * d10 + ds1b * (this->key[cnt + 2]->cv[i] - key1->cv[i]));

			res = key0->cv[i] * h1 + key1->cv[i] * h2 + dd0 * h3 + ds1 * h4;
		}
		else
			res = key0->cv[i] + t * d10;

		resVec[i] = res;
	}

}


bool Spline::load()
{
	char line[512];
	FILE *f;
	int chan;

	f = fopen(this->filename.c_str(), "rt");
	if (!f) {
		LOG->Error("Error loading spline file: %s", this->filename.c_str());
		return false;
	}

	this->key.clear();
	this->keys = 0;
	this->steps = 0;
	this->channels = 0;

	for (;;) {

		if (fgets(line, 512, f) == NULL) break;

		// comments or empty line
		if ((line[0] == ';') || (line[0] == '\n') || (line[0] == '\r')) continue;

		KeyFrame *new_key = new KeyFrame;
		chan = Util::getFloatVector(line, new_key->cv, kszKeyFrameNumChannels);

		if (chan == -1) {
			LOG->Error("Spline load error: too many floats in file: %s, line: '%s'", this->filename.c_str(), line);
			return false;
		}

		if (this->channels == 0) {
			if (chan == 0) {
				LOG->Error("Spline: incorrect format in %s", this->filename.c_str());
				return false;
			}				
			this->channels = chan;
		}
		else {
			if (this->channels != chan) {
				LOG->Error("Spline: incorrect channel in %s", this->filename.c_str());
				return false;
			}
				
		}

		new_key->tens = 0;
		new_key->cont = 0;
		new_key->bias = 0;
		new_key->linear = 0;

		this->key.push_back(new_key);
		this->keys++;

	}

	fclose(f);

	// Calculate the steps of each key based in the duration
	this->steps = this->duration;
	float motionStepTime = this->steps / (this->keys - 1);
	for (int i=0; i < this->keys; i++)
		this->key[i]->step = motionStepTime * i;

	return true;
}

/*
Motion * Spline::load(const std::string & file_name)
{
	char line[512];
	KeyFrame *key, *tmp;
	Motion *motion;
	FILE *f;
	int chan;

	f = fopen(file_name.c_str(), "rt");
	if (!f) {
		LOG->Error("Error loading spline file: %s", file_name.c_str());
		return NULL;
	}

	motion = (Motion*)malloc(sizeof(Motion));
	motion->keylist = NULL;
	motion->keys = 0;
	motion->steps = 0;
	motion->channels = 0;

	for (;;) {

		if (fgets(line, 512, f) == NULL) break;

		// comments or empty line
		if ((line[0] == ';') || (line[0] == '\n') || (line[0] == '\r')) continue;

		tmp = motion->keylist;
		motion->keylist = (KeyFrame*)malloc((motion->keys + 1) * sizeof(KeyFrame));
		memcpy(motion->keylist, tmp, motion->keys * sizeof(KeyFrame));
		free(tmp);

		key = &motion->keylist[motion->keys];
		chan = Util::getFloatVector(line, key->cv, NUM_CHAN);
		if (chan == -1)
			LOG->Error("Spline load error: too many floats in file: %s, line: '%s'", file_name.c_str(), line);

		if (motion->channels == 0) {
			if (chan == 0)
				LOG->Error("Spline: incorrect format in %s", file_name.c_str());
			motion->channels = chan;
		}
		else {
			if (motion->channels != chan)
				LOG->Error("Spline: incorrect channel in %s", file_name.c_str());
		}

		key->tens = 0;
		key->cont = 0;
		key->bias = 0;
		key->linear = 0;

		motion->keys++;
	}

	fclose(f);

	return motion;
}
*/