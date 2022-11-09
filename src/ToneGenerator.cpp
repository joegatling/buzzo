#include "ToneGenerator.h"

#define SPEAKER_PIN 15

ToneGenerator::ToneGenerator():
_isTicking(false),
_currentNote(0),
_currentDuration(0)
{
}

void ToneGenerator::DoSound(SoundId soundId, bool clearQueue)
{
    if(clearQueue)
    {
        ClearSoundQueueu();
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
        EnqueueNote(NOTE_E3, 50);
    }
    else if(soundId == ToneGenerator::TOCK)
    {
        EnqueueNote(NOTE_E2, 50);
    }
}

void ToneGenerator::StartTicking()
{
    ClearSoundQueueu();
    
    DoSound(ToneGenerator::TICK, false);

    _lastTickTime = millis();
    _isTicking = true;
    _tock = false;
}
void ToneGenerator::StopTicking()
{
    _isTicking = false;
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
            noTone(SPEAKER_PIN);
        }
    }

    if(_currentNote == 0 && _soundQueueCount > 0)
    {
        int index = _soundQueueStart;

        _currentNote = _soundQueue[index][0];
        _currentDuration = _soundQueue[index][1];

        if(_currentNote != NOTE_NONE)
        {
            tone(SPEAKER_PIN, _currentNote);
        }
        
        _soundQueueStart = (_soundQueueStart + 1) % MAX_SOUND_QUEUE;
        _soundQueueCount--;

        _lastNoteTime = millis();
    }

}

void ToneGenerator::ClearSoundQueueu()
{
    _soundQueueCount = 0;
    noTone(SPEAKER_PIN);
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