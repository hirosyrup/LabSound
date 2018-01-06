// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioDestinationNode.h"

#include "LabSound/extended/SoundBuffer.h"
#include "LabSound/extended/AudioContextLock.h"

#include "internal/AudioBus.h"
#include "internal/AudioFileReader.h"

#include <stdio.h>
#include <iostream>

namespace lab 
{

    SoundBuffer::SoundBuffer() { }
    SoundBuffer::~SoundBuffer() { }

    SoundBuffer::SoundBuffer(const char * path, float sampleRate)
    {
        initialize(path, sampleRate);
    }

    SoundBuffer::SoundBuffer(const std::vector<uint8_t> & buffer, std::string extension, float sampleRate)
    {
        initialize(buffer, extension, sampleRate);
    }

    void SoundBuffer::initialize(const char * path, float sampleRate)
    {
        std::shared_ptr<AudioBus> busForFile = MakeBusFromFile(path, false, sampleRate);
        if (auto f = busForFile.get())
        {
             audioBuffer = std::make_shared<AudioBuffer>(f);
        }
    }
    
    void SoundBuffer::initialize(const std::vector<uint8_t> & buffer, std::string extension, float sampleRate)
    {
        std::shared_ptr<AudioBus> busForFile = MakeBusFromMemory(buffer, extension, true, sampleRate);
        if (auto f = busForFile.get())
        {
             audioBuffer = std::make_shared<AudioBuffer>(f);
        }
    }

    std::shared_ptr<SampledAudioNode> SoundBuffer::create(ContextRenderLock& r, float sampleRate)
    {
        if (audioBuffer) 
        {
            std::shared_ptr<SampledAudioNode> sourceBuffer(new SampledAudioNode(sampleRate));
            // Connect the source node to the parsed audio data for playback
            sourceBuffer->setBuffer(r, audioBuffer);
            return sourceBuffer;
        }
        return nullptr;
    }
    
    // Output to the default context output 
    std::shared_ptr<SampledAudioNode> SoundBuffer::play(ContextRenderLock & r, float when)
    {
        if (audioBuffer && r.context())
        {
            return play(r, r.context()->destination(), when);
        }
        return nullptr;
    }

    // Output to a specific note 
    std::shared_ptr<SampledAudioNode> SoundBuffer::play(ContextRenderLock& r, std::shared_ptr<AudioNode> outputNode, float when)
    {
        auto ac = r.context();
        if (audioBuffer && ac) 
        {
            std::shared_ptr<SampledAudioNode> sourceBufferNode(new SampledAudioNode(outputNode->sampleRate()));
            sourceBufferNode->setBuffer(r, audioBuffer);
            
            // bus the sound to the output node
            sourceBufferNode->start(when);
            ac->connect(outputNode, sourceBufferNode, 0, 0);
            ac->holdSourceNodeUntilFinished(sourceBufferNode);
            return sourceBufferNode;
        }
        return nullptr;
    }
    
    // This variant starts a sound at a given offset relative to the beginning of the
    // sample, ends it an offset (relative to the beginning), and optional delays
    // the start. If 0 is passed as end, then the sound will play to the end.
    std::shared_ptr<SampledAudioNode> SoundBuffer::play(ContextRenderLock& r, float start, float end, float when)
    {
        auto context = r.context();
        if (audioBuffer && context)
        {
            if (end == 0.0f) end = (float) audioBuffer->duration();
            
            std::shared_ptr<SampledAudioNode> sourceBufferNode(new SampledAudioNode(context->destination()->sampleRate()));
            
            // Set the source node to the parsed audio data for playback
            sourceBufferNode->setBuffer(r, audioBuffer);
            
            // bus the sound to the mixer
            context->connect(context->destination(), sourceBufferNode, 0, 0);
            context->holdSourceNodeUntilFinished(sourceBufferNode);
            sourceBufferNode->startGrain(when, start, end - start);

            return sourceBufferNode;
        }
        return nullptr;
    }

}
