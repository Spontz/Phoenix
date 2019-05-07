#pragma comment( lib, "bass" )
#include "main.h"
#include <bass/bass.h>

#define BUFFER_SAMPLES	512
#define	DEFAULT_ENERGY	1.0f
//#define	BEAT_RATIO		1.4f
//#define FADE_OUT		4.0f

typedef struct {
	HSTREAM str;	// Music Stream

	// Beat parameters
	float	energy[BUFFER_SAMPLES];
	float	beat_ratio;
	float	fade_out;
	float	intensity;
	int		position;
	float	volume;
	float	prev_volume;	// Previous volume value
} sound_section;

static sound_section* local;

sSound::sSound()
{
	type = SectionType::Sound;
}

void sSound::load() {
	if (!DEMO->sound) {
		return;
	}

	if (this->stringNum != 1) {
		LOG->Error("Sound [%s]: 1 string needed: path to the sound file", this->identifier.c_str());
		return;
	}

	if (this->paramNum != 1) {
		LOG->Error("Sound [%s]: 1 param needed: volume of the sound (0.0 to 1.0)", this->identifier.c_str()); 
		return;
	}

	local = (sound_section*)malloc(sizeof(sound_section));
	this->vars = (void *)local;

	local->volume = this->param[0];
	local->prev_volume = local->volume;

	// Beat detection - Init variables
	local->beat_ratio = DEMO->beat_ratio;
	local->fade_out = DEMO->beat_fadeout;

	// Clean variables
	for (auto i = 0; i < BUFFER_SAMPLES; i++) {
		local->energy[i] = DEFAULT_ENERGY;
	}
	local->intensity = 0;
	local->position = 1;

	string file = DEMO->demoDir + this->strings[0];
	local->str = BASS_StreamCreateFile(FALSE, file.c_str(), 0, 0, BASS_STREAM_PRESCAN);
	if (local->str == 0)
		LOG->Error("Sound [%s]: Cannot read file: %s - Error Code: %i", this->identifier.c_str(), file.c_str(), BASS_ErrorGetCode());
}

void sSound::init() {
	int BASS_err = 0;

	if (!DEMO->sound)
		return;

	if (DEMO->state != DEMO_PLAY)
		return;

	local = (sound_section *)this->vars;

	// Beat detection - Init variables
	local->beat_ratio = DEMO->beat_ratio;
	local->fade_out = DEMO->beat_fadeout;

	// Clean variables
	for (auto i = 0; i < BUFFER_SAMPLES; i++) {
		local->energy[i] = DEFAULT_ENERGY;
	}
	local->intensity = 0;
	local->position = 1;

	QWORD bytes = BASS_ChannelSeconds2Bytes(local->str, this->runTime);		// convert seconds to bytes
	if (FALSE == BASS_ChannelSetPosition(local->str, bytes, BASS_POS_BYTE)) { // seek there
		BASS_err = BASS_ErrorGetCode();
		if (BASS_err > 0 && BASS_err != BASS_ERROR_POSITION)
			LOG->Error("Sound [%s]: BASS_ChannelSetPosition returned error: %i", this->identifier.c_str(), BASS_ErrorGetCode());
	}
	
	if (FALSE == BASS_ChannelPlay(local->str, FALSE))
		LOG->Error("Sound [%s]: BASS_ChannelPlay returned error: %i", this->identifier.c_str(), BASS_ErrorGetCode());
		
	BASS_ChannelSetAttribute(local->str, BASS_ATTRIB_VOL, local->volume);
}

void sSound::exec() {
	float instant, avg;	// Instant energy
	int i;
	float fft[BUFFER_SAMPLES] = {0.0f}; // 512 samples, because we have used "BASS_DATA_FFT1024", and this returns 512 values

	local = (sound_section *)this->vars;

	// Update local values with the ones defined by the demosystem
	local->beat_ratio = DEMO->beat_ratio;
	local->fade_out = DEMO->beat_fadeout;

	// Adjust volume if necessary
	if (local->volume != local->prev_volume) {
		BASS_ChannelSetAttribute(local->str, BASS_ATTRIB_VOL, local->volume);
		local->prev_volume = local->volume;
	}

	if (FALSE == BASS_ChannelGetData(local->str, fft, BASS_DATA_FFT1024)) {	// get the FFT data
		int BASS_err = BASS_ErrorGetCode();
		if ((BASS_err > 0) && (BASS_err != BASS_ERROR_ENDED))
			LOG->Error("Sound [%s]: BASS_ChannelGetData returned error: %i", this->identifier.c_str(), BASS_err);
	}

	instant = 0;
	for (i = 0; i < (int)BUFFER_SAMPLES; i++)
		instant += fft[i];

	// calculate average energy in last samples
	avg = 0;
	for (i = 0; i < BUFFER_SAMPLES; i++) {
		avg += local->energy[i];
	}
	avg /= (float)local->position;

	// instant sample is a beat?
	if ((instant / avg) > local->beat_ratio) {
		local->intensity = 1.0f;
	}
	else if (local->intensity > 0) {
		local->intensity -= local->fade_out * DEMO->frameTime;
		if (local->intensity < 0) local->intensity = 0;
	}

	// updated kernel shared variable
	// to be used by kernel itself or another sections
	DEMO->beat = local->intensity;

	// update energy buffer
	if (local->position < BUFFER_SAMPLES) {
		local->energy[local->position - 1] = instant;
		local->position++;
	}
	else {
		for (i = 1; i < BUFFER_SAMPLES; i++) {
			local->energy[i - 1] = local->energy[i];
		}
		local->energy[BUFFER_SAMPLES - 1] = instant;
	}

	//TODO: Draw the spectrum analyzer... it's worth it�?
	/*
	// Spectrum drawing (Only in debug mode, when the timing information is also being displayed)
	if ((DEMO->debug) && (DEMO->drawSound)) {
		glDisable(GL_DEPTH_TEST);

		if (this->hasBlend) {
			glEnable(GL_BLEND);
			glBlendFunc(mySection->sfactor, mySection->dfactor);
		}
		camera_set2d();
		glDisable(GL_TEXTURE_2D);
		glColor4f(1, 1, 1, 1);
		glBegin(GL_LINES);
		for (i = 0; i < BUFFER_SAMPLES; i++) {
			glVertex2f(i / 640.0f, fft[i]);
			glVertex2f(i / 640.0f, 0);
		}
		glEnd();

		// Draws a quad when a beat is detected
		if (local->intensity > 0) {
			glColor4f(local->intensity, local->intensity, local->intensity, local->intensity);
			//tex_bind(local->texture);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2f(0.9f, 0.9f);

			glTexCoord2f(1, 0);
			glVertex2f(1, 0.9f);

			glTexCoord2f(1, 1);
			glVertex2f(1, 1);

			glTexCoord2f(0, 1);
			glVertex2f(0.9f, 1);
			glEnd();
		}
		camera_restore();

		if (this->hasBlend) glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	*/
	
}

void sSound::end() {
	if (!DEMO->sound)
		return;

	local = (sound_section *)this->vars;

	BOOL r = BASS_ChannelStop(local->str);
	if (r != TRUE)
		LOG->Error("Sound [%s]: BASS_ChannelStop returned error: %i", this->identifier.c_str(), BASS_ErrorGetCode());
}