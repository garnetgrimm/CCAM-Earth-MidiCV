#include "adsr.h"

float Adsr::step() {
	if (state == AdsrState::DEAD) {
		return 0;
	}
	if (state_time >= target_time) {
		switch(state) {
			case AdsrState::ATTACK:
				state = AdsrState::DECAY;
				target_time = params_.decay_time;
				break;
			case AdsrState::DECAY:
				state = AdsrState::RELEASE;
				target_time = params_.release_time;
				break;
			case AdsrState::RELEASE:
				state = AdsrState::DEAD;
				target_time = 0;
				break;
			default:
				break;
		}
		state_time = 0;
	}

	float volume;
	float perc = state_time / target_time;
	switch(state) {
		case AdsrState::ATTACK:
			volume = perc;
			break;
		case AdsrState::DECAY:
			volume = ((params_.sustain_amt - 1.0) * perc) + 1.0f;
			break;
		case AdsrState::RELEASE:
			volume = ((-1.0f * params_.sustain_amt) * perc) + params_.sustain_amt;
			break;
		default:
			volume = 0.0f;
			break;
	}
	
	//1000ms tick
	state_time += 1.0f/1000.0f;
    step_ = volume;
	return volume;
}

void Adsr::trigger() {
	state_time = 0.0f;
	params_ = params;
	target_time = params_.attack_time;
	state = AdsrState::ATTACK;
}

bool Adsr::is_dead() {
	return state == AdsrState::DEAD;
}

AdsrState Adsr::get_state() {
	return state;
}

float Adsr::get_last_step() {
    return step_;
}