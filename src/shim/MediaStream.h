
#pragma once

#include "RefCounted.h"

class MediaStream : public RefCounted<MediaStream>
{
public:
    class Tracks {
    public:
        int length() const { return 2; }
    };
    
    bool isLocal() const { return true; }
    Tracks* audioTracks() { return &_tracks; }
    
private:
    Tracks _tracks;
};

class MediaStreamSource : public RefCounted<MediaStreamSource>
{
public:
};
