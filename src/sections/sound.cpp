#pragma comment( lib, "bass" )

#include <bass.h>

#include "main.h"

#define BUFFER_SAMPLES	512
#define	DEFAULT_ENERGY	1.0f
//#define	BEAT_RATIO		1.4f
//#define FADE_OUT		4.0f

struct sSound : public Section{
public:
	sSound();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	HSTREAM str;	// Music Stream

	// Beat parameters
	float	energy[BUFFER_SAMPLES];
	float	beat_ratio;
	float	fade_out;
	float	intensity;
	int		position;
	float	volume;
	float	prev_volume;	// Previous volume value
};

Section* instance_sound() {
	return new sSound();
}

sSound::sSound()
{
	type = SectionType::Sound;
}

bool sSound::load() {
	if (!DEMO->sound) {
		return false;
	}

	if (param.size() != 1 || strings.size() != 1) {
		LOG->Error("Sound [%s]: 1 param and 1 string needed: volume (0.0 to 1.0), and path to the sound file", identifier.c_str());
		return false;
	}

	volume = param[0];
	prev_volume = volume;

	// Beat detection - Init variables
	beat_ratio = DEMO->beat_ratio;
	fade_out = DEMO->beat_fadeout;

	// Clean variables
	for (auto i = 0; i < BUFFER_SAMPLES; i++) {
		energy[i] = DEFAULT_ENERGY;
	}
	intensity = 0;
	position = 1;

	std::string file = DEMO->dataFolder + strings[0];
	str = BASS_StreamCreateFile(FALSE, file.c_str(), 0, 0, BASS_STREAM_PRESCAN);
	if (str == 0) {
		LOG->Error("Sound [%s]: Cannot read file: %s - Error Code: %i", identifier.c_str(), file.c_str(), BASS_ErrorGetCode());
		return false;
	}
	return true;
}

void sSound::init() {
	int BASS_err = 0;

	if (!DEMO->sound)
		return;

	if (DEMO->state != DEMO_PLAY)
		return;

	// Beat detection - Init variables
	beat_ratio = DEMO->beat_ratio;
	fade_out = DEMO->beat_fadeout;

	// Clean variables
	for (auto i = 0; i < BUFFER_SAMPLES; i++) {
		energy[i] = DEFAULT_ENERGY;
	}
	intensity = 0;
	position = 1;

	QWORD bytes = BASS_ChannelSeconds2Bytes(str, runTime);		// convert seconds to bytes
	if (FALSE == BASS_ChannelSetPosition(str, bytes, BASS_POS_BYTE)) { // seek there
		BASS_err = BASS_ErrorGetCode();
		if (BASS_err > 0 && BASS_err != BASS_ERROR_POSITION)
			LOG->Error("Sound [%s]: BASS_ChannelSetPosition returned error: %i", identifier.c_str(), BASS_ErrorGetCode());
	}
	
	if (FALSE == BASS_Start())
		LOG->Error("Sound [%s]: BASS_Start returned error: %i", identifier.c_str(), BASS_ErrorGetCode());

	if (FALSE == BASS_ChannelPlay(str, FALSE))
		LOG->Error("Sound [%s]: BASS_ChannelPlay returned error: %i", identifier.c_str(), BASS_ErrorGetCode());
		
	BASS_ChannelSetAttribute(str, BASS_ATTRIB_VOL, volume);
}

void sSound::exec() {
	float instant, avg;	// Instant energy
	int i;
	float fft[BUFFER_SAMPLES] = {0.0f}; // 512 samples, because we have used "BASS_DATA_FFT1024", and this returns 512 values

	// Update local values with the ones defined by the demosystem
	beat_ratio = DEMO->beat_ratio;
	fade_out = DEMO->beat_fadeout;

	// Adjust volume if necessary
	if (volume != prev_volume) {
		BASS_ChannelSetAttribute(str, BASS_ATTRIB_VOL, volume);
		prev_volume = volume;
	}

	if (-1 == BASS_ChannelGetData(str, fft, BASS_DATA_FFT1024)) {	// get the FFT data
		int BASS_err = BASS_ErrorGetCode();
		if ((BASS_err > 0) && (BASS_err != BASS_ERROR_ENDED))
			LOG->Error("Sound [%s]: BASS_ChannelGetData returned error: %i", identifier.c_str(), BASS_err);
	}

	BASSDRV->copyFFTdata(fft, BUFFER_SAMPLES);

	instant = 0;
	for (i = 0; i < (int)BUFFER_SAMPLES; i++)
		instant += fft[i];

	// calculate average energy in last samples
	avg = 0;
	for (i = 0; i < BUFFER_SAMPLES; i++) {
		avg += energy[i];
	}
	avg /= (float)position;

	// instant sample is a beat?
	if ((instant / avg) > beat_ratio) {
		intensity = 1.0f;
	}
	else if (intensity > 0) {
		intensity -= fade_out * DEMO->frameTime;
		if (intensity < 0) intensity = 0;
	}

	// updated kernel shared variable
	// to be used by kernel itself or another sections
	DEMO->beat = intensity;

	// update energy buffer
	if (position < BUFFER_SAMPLES) {
		energy[position - 1] = instant;
		position++;
	}
	else {
		for (i = 1; i < BUFFER_SAMPLES; i++) {
			energy[i - 1] = energy[i];
		}
		energy[BUFFER_SAMPLES - 1] = instant;
	}
}

void sSound::end() {
	if (!DEMO->sound)
		return;

	BOOL r = BASS_ChannelStop(str);
	if (r != TRUE)
		LOG->Error("Sound [%s]: BASS_ChannelStop returned error: %i", identifier.c_str(), BASS_ErrorGetCode());
}

std::string sSound::debug() {
	std::string msg;
	msg = "[ sound id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " beat: " + std::to_string(intensity) + "\n";
	return msg;


}
