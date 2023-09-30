#ifndef PitchShiftNode_h
#define PitchShiftNode_h

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/AudioContextLock.h"

namespace lab
{
  class PitchShiftNode : public AudioNode {
      class PSOLA;
      class PitchShiftNodeInternal;
      PitchShiftNodeInternal * internalNode;
  public:
      PitchShiftNode(AudioContext &);
      virtual ~PitchShiftNode();

      static const char* static_name() { return "PitchShift"; }
      virtual const char* name() const override { return static_name(); }
      static AudioNodeDescriptor * desc();

      virtual void process(ContextRenderLock& r, int bufferSize) override;
      virtual void reset(ContextRenderLock&) override;
      virtual double tailTime(ContextRenderLock& r) const override { return 0.; }
      virtual double latencyTime(ContextRenderLock& r) const override { return 0.f; }
      void setPitch(float tone);
  };
}

#endif // PitchShiftNode_h