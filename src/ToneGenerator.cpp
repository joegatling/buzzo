#include "ToneGenerator.h"

#if defined(BUZZO_BUTTON_ALIEXPRESS)
    #define SPEAKER_PIN 15
#elif defined(BUZZO_BUTTON_ADAFRUIT)
    #define SPEAKER_PIN 9
#else
    #define SPEAKER_PIN 1
#endif

ToneGenerator::ToneGenerator():
_isTicking(false),
_currentNote(0),
_currentDuration(0),
_tickingUrgency(1)
{
    ledcDetachPin(SPEAKER_PIN);
}

void ToneGenerator::DoSound(SoundId soundId, bool clearQueue)
{
    if(clearQueue)
    {
        ClearSoundQueue();
    }

    if(soundId == ToneGenerator::BUZZ)
    {
        EnqueueNote(NOTE_G4, 50);
        EnqueueNote(NOTE_G5, 20);
        EnqueueNote(NOTE_NONE, 100);
    }
    else if(soundId == ToneGenerator::CORRECT)
    {
        EnqueueNote(NOTE_G4, 50);
        EnqueueNote(NOTE_E4, 50);
        EnqueueNote(NOTE_AS6, 50);
        EnqueueNote(NOTE_D6, 50);
        EnqueueNote(NOTE_AS5, 50);
        EnqueueNote(NOTE_D7, 50);        
    }
    else if(soundId == ToneGenerator::INCORRECT)
    {
        EnqueueNote(NOTE_G4, 20);
        EnqueueNote(NOTE_E4, 20);
        EnqueueNote(NOTE_AS4, 20);
        EnqueueNote(NOTE_D4, 20);

        EnqueueNote(NOTE_NONE, 100);

        EnqueueNote(NOTE_G4, 20);
        EnqueueNote(NOTE_E4, 20);
        EnqueueNote(NOTE_AS4, 20);
        EnqueueNote(NOTE_D4, 20);

        EnqueueNote(NOTE_NONE, 100);

        EnqueueNote(NOTE_G4, 20);
        EnqueueNote(NOTE_E4, 20);
        EnqueueNote(NOTE_AS4, 20);
        EnqueueNote(NOTE_D4, 20);
    }
    else if(soundId == ToneGenerator::CONNECTED)
    {
        EnqueueNote(NOTE_A4, 200);
        EnqueueNote(NOTE_A5, 200);        
        EnqueueNote(NOTE_A6, 200);        
    }
    else if(soundId == ToneGenerator::DISCONNECTED)
    {
        EnqueueNote(NOTE_A2, 200);
        EnqueueNote(NOTE_NONE, 20);
        EnqueueNote(NOTE_A3, 200);
        EnqueueNote(NOTE_NONE, 20);
        EnqueueNote(NOTE_A2, 200);
    }
    else if(soundId == ToneGenerator::VICTORY)
    {
        EnqueueNote(NOTE_G4, 200);
        EnqueueNote(NOTE_C5, 200);
        EnqueueNote(NOTE_E5, 200);
        EnqueueNote(NOTE_G5, 200);
        EnqueueNote(NOTE_NONE, 100);
        EnqueueNote(NOTE_E4, 100);
        EnqueueNote(NOTE_G5, 400);
    }
    else if(soundId == ToneGenerator::RESPOND)
    {
        EnqueueNote(NOTE_E4, 25);
        EnqueueNote(NOTE_E5, 25);
        EnqueueNote(NOTE_E4, 25);
        EnqueueNote(NOTE_E5, 25);
    }    
    else if(soundId == ToneGenerator::TICK)
    {
        EnqueueNote(60 + (75 * _tickingUrgency), 50);
    }
    else if(soundId == ToneGenerator::TOCK)
    {
        EnqueueNote(140 + (75 * _tickingUrgency), 50);
    }
    else if(soundId == ToneGenerator::ACKNOWLEDGE)
    {
        EnqueueNote(NOTE_E2, 20);
        EnqueueNote(NOTE_E3, 20);
    } 
    else if(soundId == ToneGenerator::POWER_ON)
    {
        EnqueueNote(NOTE_E4, 50);
        EnqueueNote(NOTE_G4, 50);
        EnqueueNote(NOTE_E5, 50);
        EnqueueNote(NOTE_G5, 50);
    }
    else if(soundId == ToneGenerator::POWER_OFF)
    {
        EnqueueNote(NOTE_G5, 50);
        EnqueueNote(NOTE_E5, 50);
        EnqueueNote(NOTE_G4, 50);
        EnqueueNote(NOTE_E4, 50);
    }

}

void ToneGenerator::StartTicking()
{
    ClearSoundQueue();
    
    DoSound(ToneGenerator::TICK, false);

    _lastTickTime = millis();
    _isTicking = true;
    _tock = true;
}
void ToneGenerator::StopTicking()
{
    _isTicking = false;
}

void ToneGenerator::SetTickingUrgency(float urgency)
{
    _tickingUrgency = urgency;

    if(_tickingUrgency < 0)
    {
        _tickingUrgency = 0;
    }
    else if(_tickingUrgency > 1)
    {
        _tickingUrgency = 1;
    }
}

void ToneGenerator::Update()
{
    if(_isTicking)
    {
        if(millis() - _lastTickTime > 1000)
        {
            _lastTickTime += 1000;

            DoSound(_tock ? TOCK : TICK, false);
            _tock = !_tock;
        }
    }

    if(_currentNote > 0)
    {
        if(millis() - _lastNoteTime > _currentDuration)
        {
            _currentNote = 0;
            //noTone(SPEAKER_PIN);
            ledcDetachPin(SPEAKER_PIN);
        }
    }

    if(_currentNote == 0 && _soundQueueCount > 0)
    {
        int index = _soundQueueStart;

        _currentNote = _soundQueue[index][0];
        _currentDuration = _soundQueue[index][1];

        if(_currentNote != NOTE_NONE && _currentNote >= NOTE_MIN && _currentNote <= NOTE_MAX)
        {
            ledcAttachPin(SPEAKER_PIN, 0);
            ledcWriteTone(0, _currentNote);
        }
        
        _soundQueueStart = (_soundQueueStart + 1) % MAX_SOUND_QUEUE;
        _soundQueueCount--;

        _lastNoteTime = millis();
    }

}

bool ToneGenerator::IsPlayingSound()
{
    return _soundQueueCount > 0 || _currentNote > NOTE_NONE;
}

void ToneGenerator::ClearSoundQueue()
{
    _soundQueueCount = 0;
    //noTone(SPEAKER_PIN);
    ledcDetachPin(SPEAKER_PIN);
}

void ToneGenerator::EnqueueNote(unsigned int note, unsigned int duration)
{
    if(_soundQueueCount < MAX_SOUND_QUEUE)
    {
        int index = (_soundQueueStart + _soundQueueCount) % MAX_SOUND_QUEUE;

        _soundQueue[index][0] = note;
        _soundQueue[index][1] = duration;

        _soundQueueCount++;
    }
}

