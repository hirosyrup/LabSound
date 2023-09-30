#include "LabSound/core/PitchShiftNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"
#include <samplerate.h>
#include <cstring>
#include <SoundTouch.h>

namespace lab 
{

  static AudioParamDescriptor s_pitchShiftParams[] = {{"pitch", "Pitch", 0.0, -1200.0, 1200.0}, nullptr};
  AudioNodeDescriptor * PitchShiftNode::desc()
  {
      static AudioNodeDescriptor d {s_pitchShiftParams, nullptr};
      return &d;
  }

  class PitchShiftNode::PSOLA {
    private:
      soundtouch::SoundTouch mSoundTouch;
      float mTone;

    public:

      PSOLA() {
          mSoundTouch.setChannels(1);
          mSoundTouch.setPitchSemiTones(0.0);
          mTone = 0.0;
      }

      void process(const float* src, float* dst, int bufferSize) {
          if (mTone == 0.0) {
            std::memcpy(dst, src, sizeof(float) * bufferSize);
            return;
          }

          mSoundTouch.putSamples(src, bufferSize);
          mSoundTouch.receiveSamples(dst, bufferSize);
      }

      void setPitch(float tone) {
        mSoundTouch.setPitchSemiTones(tone);
        mTone = tone;
      }

      void setSampleRate(float sampleRate) {
        mSoundTouch.setSampleRate(sampleRate);
      }

      void reset() {
        setPitch(0.0);
      }
  };

  class PitchShiftNode::PitchShiftNodeInternal : public AudioProcessor {

  public:
      PSOLA m_shiftL;
      PSOLA m_shiftR;

      PitchShiftNodeInternal() : AudioProcessor() {
      }

      virtual ~PitchShiftNodeInternal() {}

      virtual void initialize() override { }
      virtual void uninitialize() override { }

      virtual void process(ContextRenderLock & r, const AudioBus * source, AudioBus * destination, int bufferSize) override {
          if (!source || !destination)
              return;

          const float* srcL = source->channel(0)->data();
          float* destL = destination->channel(0)->mutableData();

          m_shiftL.process(srcL, destL, bufferSize);

          if (source->numberOfChannels() > 1 && destination->numberOfChannels() > 1) {
              const float* srcR = source->channel(1)->data();
              float* destR = destination->channel(1)->mutableData();

              m_shiftR.process(srcR, destR, bufferSize);
          }
      }

      virtual void reset() override { 
        m_shiftL.reset();
        m_shiftR.reset();
      }

      virtual double tailTime(ContextRenderLock & r) const override { return 0; }
      virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
  };

  PitchShiftNode::PitchShiftNode(AudioContext& ac) 
      : AudioNode(ac, *desc())
      , internalNode(new PitchShiftNodeInternal) {
      addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
      addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));

      const float sampleRate = ac.sampleRate();
      internalNode->m_shiftL.setSampleRate(sampleRate);
      internalNode->m_shiftR.setSampleRate(sampleRate);

      initialize();
  }

  PitchShiftNode::~PitchShiftNode() {
      uninitialize();
      delete internalNode;
  }

  void PitchShiftNode::setPitch(float tone) {
      internalNode->m_shiftL.setPitch(tone);
      internalNode->m_shiftR.setPitch(tone);
  }

  void PitchShiftNode::process(ContextRenderLock& r, int bufferSize) {
      AudioBus* destinationBus = output(0)->bus(r);
      AudioBus* sourceBus = input(0)->bus(r);
      
      if (!isInitialized() || !input(0)->isConnected())
      {
          destinationBus->zero();
          return;
      }

      int numberOfInputChannels = input(0)->numberOfChannels(r);
      if (numberOfInputChannels != output(0)->numberOfChannels())
      {
          output(0)->setNumberOfChannels(r, numberOfInputChannels);
          destinationBus = output(0)->bus(r);
      }

      internalNode->process(r, sourceBus, destinationBus, bufferSize);
  }

  void PitchShiftNode::reset(ContextRenderLock&) {
      internalNode->reset();
  }
}