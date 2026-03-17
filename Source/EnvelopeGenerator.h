#pragma once
#include <cmath>

struct EnvelopeGenerator
{
    EnvelopeGenerator() :
		state(State::Release),
        sr(44100.f),
        attackSamples(0.f),
        releaseSamples(0.f),
        atk(1.f),
        rls(1.f),
        env(0.f)
    {
	}

    void prepare(float sampleRate)
    {
        sr = sampleRate;
    }

    void setAttack(float attackMs)
    {
        attackSamples = (attackMs / 1000.0f) * sr;
		atk = 1.f / attackSamples;
    }

    void setRelease(float releaseMs)
    {
        releaseSamples = (releaseMs / 1000.0f) * sr;
		rls = 1.f / releaseSamples;
    }

    void trigger() noexcept
    {
        state = State::Attack;
    }

    void release() noexcept
    {
        state = State::Release;
    }

    float operator()() noexcept
    {
        switch (state)
        {
        case State::Attack:
            env += atk;
            if (env >= 1.f)
            {
                env = 1.f;
                state = State::Sustain;
            }
			break;
		case State::Release:
			env -= rls;
			if (env <= 0.f)
            {
                env = 0.f;
            }
            break;
        }
		return env;
    }

    bool isActive() const noexcept
    {
        return state != State::Release || env > 0.f;
    }
private:
    enum class State { Attack, Sustain, Release };
    State state;
    float sr, attackSamples, releaseSamples, atk, rls, env;
};
