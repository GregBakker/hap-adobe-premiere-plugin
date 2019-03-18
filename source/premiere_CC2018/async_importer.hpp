#ifndef ASYNC_IMPORTER
#define ASYNC_IMPORTER

#include <future>
#include <mutex>

#include "importer.hpp"


// async_importer.hpp
//   support classes for async import

// Track an async frame read and decode
struct AsyncFrameRequest
{
    AsyncFrameRequest(int32_t frameNum_,
                      PPixHand adobeFrame_   // !!! hack
                     )
        : frameNum(frameNum_), adobeFrame(adobeFrame_)
    {
    }
    int32_t frameNum;

    PPixHand adobeFrame;
    std::promise<PPixHand> adobeFramePromise;
};

PREMPLUGENTRY xAsyncImportEntry(int inSelector, void *inParam);


// This object is created by an importer during imCreateAsyncImporter
class AsyncImporter
{
public:
    AsyncImporter(
        std::unique_ptr<AdobeImporterAPI> adobe,
        std::unique_ptr<Importer> importer,
        int32_t width, int32_t height,
        int32_t frameRateNumerator, int32_t frameRateDenominator);

    int32_t convertTimeToFrame(double t) const;

    ~AsyncImporter();
    int OnInitiateAsyncRead(imSourceVideoRec& inSourceRec);
    int OnFlush();
    int OnGetFrame(imSourceVideoRec* inFrameRec);
private:
    void serviceFailedRequests();

    std::unique_ptr<AdobeImporterAPI> adobe_;
    std::unique_ptr<Importer> importer_;

    int32_t width_;
    int32_t height_;
    int32_t frameRateNumerator_;
    int32_t frameRateDenominator_;

    std::mutex requestsLock_;
    std::vector<AsyncFrameRequest *> requests_;

    std::mutex failedRequestsLock_;
    std::vector<AsyncFrameRequest *> failedRequests_;
};

#endif