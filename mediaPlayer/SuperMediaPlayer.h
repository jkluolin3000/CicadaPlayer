#ifndef CICADA_PLAYER_SERVICE_H
#define CICADA_PLAYER_SERVICE_H

#include <string>

using namespace std;

#include "native_cicada_player_def.h"
#include "demuxer/demuxer_service.h"

#include "MediaPlayerUtil.h"

#include "player_msg_control.h"
#include "buffer_controller.h"

#include <deque>
#include "system_refer_clock.h"

#include "SMPAVDeviceManager.h"
#include "SMPMessageControllerListener.h"
#include "SMP_DCAManager.h"
#include "SuperMediaPlayerDataSourceListener.h"
#include "player_notifier.h"
#include "player_types.h"
#include "render/video/IVideoRender.h"
#include <filter/IAudioFilter.h>
#include <queue>
#include <render/audio/IAudioRender.h>
#include <utils/bitStreamParser.h>

#include "CicadaPlayerPrototype.h"
#include <cacheModule/CacheModule.h>
#include <cacheModule/cache/CacheConfig.h>
#include <codec/IDecoder.h>
#include <filter/FilterManager.h>

#ifdef __APPLE__

#include <TargetConditionals.h>

#endif

#include "mediaPlayerSubTitleListener.h"
#include "SMPRecorderSet.h"

namespace Cicada {
    typedef struct streamTime_t {
        int64_t startTime;
        int64_t deltaTime;
        int64_t deltaTimeTmp;

    } streamTime;

    typedef enum RENDER_RESULT {
        RENDER_NONE,
        RENDER_PARTLY,
        RENDER_FULL,
    } RENDER_RESULT;

    typedef enum APP_STATUS {
        APP_FOREGROUND,
        APP_BACKGROUND,
    } APP_STATUS;

    enum class ViewUpdateStatus {
        Unknown,
        No,
        Yes,
    };

    enum class LiveTimeSyncType {
        LiveTimeSyncNormal,
        LiveTimeSyncCatchUp,
        LiveTimeSyncSlowDown
    };

    const static float MIN_SPEED = 0.5;
    const static float MAX_SPEED = 5;

    class SuperMediaPlayer : public ICicadaPlayer, private CicadaPlayerPrototype {

        friend class SuperMediaPlayerDataSourceListener;
        friend class SMP_DCAManager;
        friend class SMPMessageControllerListener;

    public:

        SuperMediaPlayer();

        ~SuperMediaPlayer() override;

        string getName() override
        {
            return "SuperMediaPlayer";
        }

        int SetListener(const playerListener &Listener) override;

        void SetOnRenderCallBack(onRenderFrame cb, void *userData) override;

        void SetAudioRenderingCallBack(onRenderFrame cb, void *userData) override;

        void SetVideoRenderingCallBack(videoRenderingFrameCB cb, void *userData) override;

        void SetUpdateViewCB(UpdateViewCB cb, void *userData) override;

        // TODO: use setParameters and setOpt to set
        void SetRefer(const char *refer) override;

        void SetUserAgent(const char *userAgent) override;

        void SetTimeout(int timeout) override;

        void SetDropBufferThreshold(int dropValue) override;

        void SetLooping(bool looping) override;

        bool isLooping() override;

        int SetOption(const char *key, const char *value) override;

        void GetOption(const char *key, char *value) override;

        int64_t GetPlayingPosition() override
        {
            return getCurrentPosition() / 1000;
        };

        int64_t GetBufferPosition() override;

        int64_t GetDuration() const override;

        PlayerStatus GetPlayerStatus() const override;

        void SetScaleMode(ScaleMode mode) override;

        ScaleMode GetScaleMode() override;

        void SetRotateMode(RotateMode mode) override;

        RotateMode GetRotateMode() override;

        void SetMirrorMode(MirrorMode mode) override;

        void SetVideoBackgroundColor(uint32_t color) override;

        MirrorMode GetMirrorMode() override;

        int GetCurrentStreamIndex(StreamType type) override;

        StreamInfo *GetCurrentStreamInfo(StreamType type) override;

        float GetVolume() const override;

        void CaptureScreen() override;

        void SetDecoderType(DecoderType type) override;

        DecoderType GetDecoderType() override;

        bool IsMute() const override;

        float GetVideoRenderFps() override;

        float GetVideoDecodeFps() override;

        void GetVideoResolution(int &width, int &height) override;

        void GetVideoRotation(int &rotation) override;

        void SetView(void *view) override;

        void ClearScreen() override;

        void SetDataSource(const char *url) override;

        void setBitStreamCb(readCB read, seekCB seek, void *arg) override;

        void Prepare() override;

        void SetVolume(float volume) override;

        void Start() override;

        void Pause() override;

        void SeekTo(int64_t pos, bool bAccurate) override;

        void Mute(bool bMute) override;

        void EnterBackGround(bool back) override;

        StreamType SwitchStream(int streamIndex) override;

        int Stop() final;

        void setSpeed(float speed) override;

        void AddCustomHttpHeader(const char *httpHeader) override;

        void RemoveAllCustomHttpHeader() override;

        float getSpeed() override;

        void Interrupt(bool inter);

        std::string GetPropertyString(PropertyKey key, const CicadaJSONItem &param) override;

        int64_t GetPropertyInt(PropertyKey key) override;

        int64_t GetMasterClockPts() override;

        int getCurrentStreamMeta(Stream_meta *meta, StreamType type) override;

        void reLoad() override;

        void SetAutoPlay(bool bAutoPlay) override;

        bool IsAutoPlay() override;

        void SetFilterConfig(const std::string &filterConfig) override;

        void UpdateFilterConfig(const std::string &target, const std::string &options) override;

        void SetFilterInvalid(const std::string &target, bool invalid) override;

        void addExtSubtitle(const char *uri) override;

        int selectExtSubtitle(int index, bool bSelect) override;

        int setStreamDelay(int index, int64_t time) override;

        int invokeComponent(std::string content) override;

        void setDrmRequestCallback(const std::function<DrmResponseData*(const DrmRequestParam& drmRequestParam)>  &drmCallback) override;

        float getCurrentDownloadSpeed() override;

    private:
        void NotifyPosition(int64_t position);

        void NotifyUtcTime();

        void OnTimer(int64_t curTime);

        int updateLoopGap();

        int mainService();

        bool NeedDrop(int64_t pts, int64_t refer);

        void NotifyError(int code);

        void putMsg(PlayMsgType type, const MsgParam &param, bool trigger = true);

        void ProcessVideoLoop();

        void OnDemuxerCallback(const std::string &key, const std::string &value);

        bool DoCheckBufferPass();

        int DecodeVideoPacket(unique_ptr<IAFPacket> &pVideoPacket);

        void LiveCatchUp(int64_t delayTime);

        void LiveTimeSync(int64_t delayTime);

        int FillVideoFrame();

    private:
        int SetUpAudioPath();

        int setUpAudioDecoder(const Stream_meta *meta);

        int SetUpVideoPath();

        void SendVideoFrameToRender(unique_ptr<IAFFrame> frame, bool valid = true);

        // RENDER_NONE: doesn't render
        // RENDER_PARTLY: render part of data
        // RENDER_FULL: render a frame fully
        RENDER_RESULT RenderAudio();

        int DecodeAudio(unique_ptr<IAFPacket> &pPacket);

        int ReadPacket();

        void PostBufferPositionMsg();

        void ChangePlayerStatus(PlayerStatus newStatus);

        void ResetSeekStatus();

        void RenderCallback(StreamType type, bool rendered, IAFFrame::AFFrameInfo &info);

        void Reset();

        void FlushSubtitleInfo();

        void FlushAudioPath();

        void FlushVideoPath();

        bool SeekInCache(int64_t pos);

        void SwitchVideo(int64_t startTime);

        int64_t getPlayerBufferDuration(bool gotMax, bool internal);

        void ProcessOpenStreamInit(int streamIndex);

        static int64_t getAudioPlayTimeStampCB(void *arg);

        int64_t getAudioPlayTimeStamp();

        bool doFilter(unique_ptr<IAFFrame> &frame);

        bool render();

        void RenderSubtitle(int64_t pts);

        bool RenderVideo(bool force_render);

        void releaseStreamInfo(const StreamInfo *info) const;

        // mSeekFlag will be set when processing (after remove from mMessageControl), it have gap
        bool isSeeking()
        {
            return INT64_MIN != mSeekPos;
        } //{return mSeekFlag || mMessageControl.findMsgByType(MSG_SEEKTO);}

//        void setRotationMode(RotateMode rotateMode, MirrorMode mirrorMode) const;

        bool CreateVideoRender(uint64_t flags);

        int CreateVideoDecoder(bool bHW, Stream_meta &meta);

        int64_t getCurrentPosition();

        void checkEOS();

        bool checkEOSAudio();

        bool checkEOSVideo();

        void playCompleted();

        void notifySeekEndCallback();

        void notifyPreparedCallback();

        void updateVideoMeta();

        void doDeCode();

        void doRender();

        void doReadPacket();

        int setUpAudioRender(const IAFFrame::audioInfo &info);

        std::atomic<int64_t> mCurrentPos{};
        std::atomic<int64_t> mCurrentFrameUtcTime{};

        void printTimePosition(int64_t time) const;

        void setUpAVPath();

        void startRendering(bool start);

        void sendDCAMessage();

        void ProcessUpdateView();

        static bool isHDRVideo(const Stream_meta *meta);

        static bool isWideVineVideo(const Stream_meta *meta);

        void closeAudio();

        void closeVideo();

        void checkFirstRender();

        int setUpVideoRender(uint64_t renderFlags);

        class ApsaraAudioRenderCallback : public IAudioRenderListener {
        public:
            explicit ApsaraAudioRenderCallback(SuperMediaPlayer &player) : mPlayer(player)
            {}

            void onEOS() override
            {}
            void onFrameInfoUpdate(IAFFrame::AFFrameInfo &info, bool rendered) override;

        private:
            SuperMediaPlayer &mPlayer;
        };

        class ApsaraVideoRenderListener : public IVideoRender::IVideoRenderListener {

        public:
            explicit ApsaraVideoRenderListener(SuperMediaPlayer &player) : mPlayer(player)
            {}
            void onFrameInfoUpdate(IAFFrame::AFFrameInfo &info, bool rendered) override;

        private:
            SuperMediaPlayer &mPlayer;
        };

        class ApsaraVideoProcessTextureCallback : public IVideoRender::videoProcessTextureCb {
        public:
            explicit ApsaraVideoProcessTextureCallback(SuperMediaPlayer &player) : mPlayer(player)
            {}

            bool init(int type) override;

            bool needProcess() override;

            bool processTexture(std::unique_ptr<IAFFrame> &textureFrame) override;

        private:
            SuperMediaPlayer &mPlayer;
        };

    private:
        static IVideoRender::Scale convertScaleMode(ScaleMode mode);

        static IVideoRender::Rotate convertRotateMode(RotateMode mode);

        static IVideoRender::Flip convertMirrorMode(MirrorMode mode);


    public:
        static bool is_supported(const options *opts)
        {
            return true;
        }

    private:
        explicit SuperMediaPlayer(int dummy)
        {
            mIsDummy = true;
            addPrototype(this);
        }
        ICicadaPlayer *clone() override
        {
            return new SuperMediaPlayer();
        };

        static SuperMediaPlayer se;


    private:
        IDataSource *mDataSource{nullptr};
        std::atomic_bool mCanceled{false};
        std::atomic_bool mMainServiceCanceled{true};
        std::unique_ptr<demuxer_service> mDemuxerService{nullptr};
        std::queue<unique_ptr<IAFFrame>> mVideoFrameQue{};
        std::deque<unique_ptr<IAFFrame>> mAudioFrameQue{};
        unique_ptr<streamMeta> mCurrentVideoMeta{};
        bool videoDecoderEOS = false;
        bool audioDecoderEOS = false;
        picture_cache_type mPictureCacheType = picture_cache_type_cannot;
        bool videoDecoderFull = false;
        std::unique_ptr<SMPMessageControllerListener> mMsgCtrlListener{nullptr};
        std::unique_ptr<PlayerMessageControl> mMessageControl{nullptr};
        std::unique_ptr<ApsaraAudioRenderCallback> mAudioRenderCB{nullptr};
        std::unique_ptr<ApsaraVideoRenderListener> mVideoRenderListener{nullptr};
        std::unique_ptr<ApsaraVideoProcessTextureCallback> mVideoProcessCb{nullptr};
        std::unique_ptr<BufferController> mBufferController{nullptr};
        std::mutex mAppStatusMutex;
        std::atomic<APP_STATUS> mAppStatus{APP_FOREGROUND};
        int mVideoWidth{0};
        int mVideoHeight{0};
        int mVideoRotation{0};
        int64_t mDuration{INT64_MIN};
        int64_t mBufferPosition{0};
        PlayerStatus mOldPlayStatus{PLAYER_IDLE};
        atomic <PlayerStatus> mPlayStatus{PLAYER_IDLE};
        std::deque<std::unique_ptr<IAFPacket>> mSubtitleShowedQueue;
        MediaInfo mMediaInfo{};
        //        ResolutionPolicy mResolutionPolicy{kShowAll};
        int mCurrentVideoIndex{-1};
        int mCurrentAudioIndex{-1};
        int mCurrentSubtitleIndex{-1};
        int mWillChangedVideoStreamIndex{-1};
        int mWillChangedAudioStreamIndex{-1};
        int mWillChangedSubtitleStreamIndex{-1};
        float mCATimeBase{};      // current audio stream origin pts time base
        float mWATimeBase{};      // willChange audio stream origin pts time base
        int mRemainLiveSegment{0};// To avoid access demuxer multi-thread
        bool mInited{false};
        atomic_bool mSeekNeedCatch{false};
        const static int64_t SEEK_ACCURATE_MAX;
        atomic <int64_t> mSeekPos{INT64_MIN};
        SystemReferClock mMasterClock;
        streamTime mAudioTime{INT64_MIN, 0};
        int64_t mPlayedVideoPts{INT64_MIN}; // sync pts
        bool mVideoPtsRevert{false};
        bool mAudioPtsRevert{false};
        bool mHaveVideoPkt{false};
        bool mHaveAudioPkt{false};
        int64_t mPlayedAudioPts{INT64_MIN};
        int64_t mFirstVideoPts{INT64_MIN};
        int64_t mCurVideoPts{INT64_MIN};  // update from render cb
        int64_t mFirstAudioPts{INT64_MIN};
        int64_t mMediaStartPts{INT64_MIN}; // the first small frame pts in the media stream
        int64_t mVideoChangedFirstPts{INT64_MIN};
        int64_t mAudioChangedFirstPts{INT64_MIN};
        int64_t mSubtitleChangedFirstPts{INT64_MIN};
        int64_t mFirstReadPacketSucMS{0};
        int mMainStreamId{-1};
        int64_t mRemovedFirstAudioPts{INT64_MIN};;
        int64_t mFirstSeekStartTime{0};
        bool mEof{false};
        bool mSubtitleEOS{false};
        bool mLowMem{false};
        bool mSeekFlag{false};
        bool mSeekInCache{false};
        bool mFirstBufferFlag{true}; // first play and after seek play
        bool mBufferingFlag{false};
        bool mMixMode{false};
        bool mAdaptiveVideo{false};
        bool mFirstRendered{false};
        int mWriteAudioLen{0};
        int64_t mLastAudioFrameDuration{INT64_MIN};
        int64_t mTimeoutStartTime{INT64_MIN};
        int64_t mSubtitleShowIndex{0};
        bool mBufferIsFull{false};
        bool mWillSwitchVideo{false};
        std::unique_ptr<player_type_set> mSet{};
        int64_t mSoughtVideoPos{INT64_MIN};
        int mTimerInterval = 0;
        int64_t mTimerLatestTime = 0;
        std::mutex mCreateMutex{}; // need lock if access pointer outside of loop thread
        std::mutex mPlayerMutex{};
        std::mutex mSleepMutex{};
        std::condition_variable mPlayerCondition;
        PlayerNotifier *mPNotifier = nullptr;
        std::unique_ptr<afThread> mApsaraThread{};
        int mLoadingProcess{0};
        int64_t mPrepareStartTime = 0;
        int mVideoParserTimes = 0;
        InterlacedType mVideoInterlaced = InterlacedType_UNKNOWN;
        bitStreamParser *mVideoParser = nullptr;
        int64_t mPtsDiscontinueDelta{INT64_MIN};
        std::unique_ptr<MediaPlayerUtil> mUtil{};
        std::unique_ptr<SuperMediaPlayerDataSourceListener> mSourceListener{nullptr};
        std::unique_ptr<SMP_DCAManager> mDcaManager{nullptr};
        std::unique_ptr<SMPAVDeviceManager> mAVDeviceManager{nullptr};
        std::unique_ptr<IAFPacket> mVideoPacket{};
        std::unique_ptr<IAFPacket> mAudioPacket{};
        std::unique_ptr<mediaPlayerSubTitleListener> mSubListener;
        std::unique_ptr<subTitlePlayer> mSubPlayer;
        std::mutex mFilterManagerMutex{};
        std::unique_ptr<FilterManager> mFilterManager;
        bool dropLateVideoFrames = false;
        bool waitingForStart = false;
        bool mBRendingStart {false};
        bool mSecretPlayBack{false};
        bool mDrmKeyValid{false};
        std::unique_ptr<SMPRecorderSet> mRecorderSet{nullptr};
        bool mAutoPlay = false;
        int64_t mCheckAudioQueEOSTime{INT64_MIN};
        uint64_t mAudioQueDuration{UINT64_MAX};
        onRenderFrame mFrameCb{nullptr};
        void *mFrameCbUserData{nullptr};
        UpdateViewCB mUpdateViewCB{nullptr};
        void *mUpdateViewCBUserData{nullptr};
        onRenderFrame mAudioRenderingCb{nullptr};
        void *mAudioRenderingCbUserData{nullptr};

        videoRenderingFrameCB mVideoRenderingCb{nullptr};
        void *mVideoRenderingCbUserData{nullptr};

        bool mIsDummy{false};
        readCB mBSReadCb = nullptr;
        seekCB mBSSeekCb = nullptr;
        void *mBSCbArg = nullptr;
        int64_t mSuggestedPresentationDelay = 0;
        LiveTimeSyncType mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncNormal;
        bool mVideoCatchingUp{false};
        bool mAudioEOS{false};
        bool mVideoEOS{false};
        int64_t mVideoDelayTime{0};
        bool mCalculateSpeedUsePacket{true};
        std::unique_ptr<CicadaJSONArray> mFilterConfig;
        UTCTimer *mUtcTimer{nullptr};
    };
}// namespace Cicada
#endif// CICADA_PLAYER_SERVICE_H
