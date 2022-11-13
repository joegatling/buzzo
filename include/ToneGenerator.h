#pragma once

#include <Arduino.h>
#include "pitches.h"

#define MAX_SOUND_QUEUE 64

class ToneGenerator
{
    public:
    enum SoundId
    {
        BUZZ,
        CORRECT,
        INCORRECT,
        DISCONNECTED,
        CONNECTED,
        VICTORY,
        RESPOND,
        TICK,
        TOCK
    };

    ToneGenerator();

    void DoSound(SoundId soundId, bool clearQueue = true);

    void StartTicking();
    void StopTicking();
    void SetTickingUrgency(float urgency);

    void Update();

    private:

    void ClearSoundQueueu();

    void EnqueueNote(unsigned int note, unsigned int duration);
    
    unsigned long _lastTickTime = 0;
    bool _isTicking = false;
    bool _tock = false;
    float _tickingUrgency;

    unsigned long _lastNoteTime = 0;

    unsigned int _soundQueue[MAX_SOUND_QUEUE][2];
    int _soundQueueCount = 0;
    int _soundQueueStart = 0;

    unsigned int _currentNote;
    unsigned int _currentDuration;
};