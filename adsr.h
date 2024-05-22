#pragma once

enum class AdsrState {
	ATTACK = 0,
	DECAY = 1,
	RELEASE = 2,
	DEAD = 3,
};

struct AdsrParams {
	float attack_time;
	float decay_time;
	float sustain_amt;
	float release_time;
};

class Adsr {
	AdsrState state = AdsrState::DEAD;
	AdsrParams params_;
    float step_;
public:
	float state_time = 0;
	float target_time = 0;
	AdsrParams params = { 1.0f, 1.0f, 1.0f, 1.0f };
	float step();
	void trigger();
	bool is_dead();
    float get_last_step();
	AdsrState get_state();

};