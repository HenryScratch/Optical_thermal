

#ifndef OBJECT_TRACKER_TRACKER_LOG_H
#define OBJECT_TRACKER_TRACKER_LOG_H

#include <unordered_map>
#include <vector>
#include <ostream>

namespace OT {
    struct Track {
        int trackerId;
        int x;
        int y;
        long frameNumber;
    };

    class TrackerLog {
    private:
        // The maximum frame seen in any track so far.
        long numFrames;

        // Whether the JSON output should be compressed (i.e. no spaces, no keys for tracks).
        bool compress;

        // Capture the dimensions of the frame.
        int width = -1;
        int height = -1;
    public:
        explicit TrackerLog(bool compress = false);

        // Update the tracker log with the latest info for some tracker.
        void addTrack(int trackerId, int x, int y, long frameNumber);

        // Output the log to the given file as JSON.
        void logToStream(std::ostream& outputStream);

        // Set the frame dimensions.
        void setDimensions(int _width, int _height);

        // Get the tracks associated with a tracker given its tracker id.
        std::unordered_map<int, std::vector<Track>> tracksForTrackerId;

        // Given a tracker ID, return the first frame that the tracker appears in.
        std::unordered_map<int, long> birthFrameForTrackerId;
    };
}



#endif //OBJECT_TRACKER_TRACKER_LOG_H
