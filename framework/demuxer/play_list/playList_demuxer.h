//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PLAYLIST_DEMUXER_H
#define FRAMEWORK_PLAYLIST_DEMUXER_H

#include <string>
#include "data_source/proxyDataSource.h"

using namespace std;

#include "PlaylistManager.h"
#include "demuxer/IDemuxer.h"
#include "demuxer/demuxerPrototype.h"
#include "playListParser.h"
#include "utils/xml/DOMParser.h"

namespace Cicada{

    class playList_demuxer : public IDemuxer, private demuxerPrototype {

    public:
        enum playList_type {
            playList_type_unknown = 0,
            playList_type_hls,
            playList_type_dash
        };

        explicit playList_demuxer();

        playList_demuxer(const string& path, playList_type type);

        ~playList_demuxer() override;

        const playList *GetPlayList() override
        {
            return mPPlayList;
        }

        int Open() override;

        int ReadPacket(std::unique_ptr<IAFPacket> &packet, int index) override;

        void Close() override;

        void Start() override;

        void Stop() override;

        void PreStop() override;

        int64_t Seek(int64_t us, int flags, int index) override;

        int GetNbStreams() const override;

        int GetNbSubStreams(int index) const override;

        int GetRemainSegmentCount(int index) override;
        
        int GetSourceMeta(Source_meta **meta) const override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) const override;

        int GetMediaMeta(Media_meta *mediaMeta) const override;

        int OpenStream(int index) override;

        void CloseStream(int index) override;

        void interrupt(int inter) override;

        int SwitchStreamAligned(int from, int to) override;

        int64_t getMaxGopTimeUs() override;

        UTCTimer *getUTCTimer() override;

        void setClientBufferLevel(client_buffer_level level) override;

        int SetOption(const std::string &key, const int64_t value) override;

        void flush() override
        {
            // TODO:
        };

        bool isPlayList() const override
        {
            return true;
        }

        const std::string GetProperty(int index, const string &key) const override;
        
        bool isRealTimeStream(int index) override;
        bool isWallclockTimeSyncStream(int index) override;
        int64_t getDurationToStartStream(int index) override;

        bool isTSDiscontinue() override
        {
            return true;
        }

        int64_t getBufferDuration(int index) const override;

        void setUrlToUniqueIdCallback(UrlHashCB cb, void *userData) override;
        
        void setM3u8DecryptKeyCallBack(m3u8_decrypt_key_callback m3u8DeckeyCb, void *userData) override;

    private:
        explicit playList_demuxer(int dummy) : IDemuxer("")
        {
            addPrototype(this);
            // need to call xmlInitParser() in the "main" thread before using any of the libxml2 API
            // see http://www.xmlsoft.org/threads.html
            Cicada::DOMParser::InitXml();
        }

        Cicada::IDemuxer *clone(const string &uri, int type, const Cicada::DemuxerMeta *meta) override
        {
            return new playList_demuxer(uri, static_cast<playList_type>(type));
        }

        bool is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type, const Cicada::DemuxerMeta *meta,
                          const Cicada::options *opts) override;

        int getType() override
        {
            return demuxer_type_playlist;
        }

        static playList_demuxer se;

    private:

        playList *mPPlayList = nullptr;
        playListParser *mParser = nullptr;
        playList_type mType{playList_type_unknown};
        PlaylistManager *mPPlaylistManager = nullptr;
        proxyDataSource *mProxySource = nullptr;

        int64_t mFirstSeekPos = INT64_MIN;
        UrlHashCB mUrlHashCb{nullptr};
        void *mUrlHashCbUserData{nullptr};
        //
        m3u8_decrypt_key_callback mM3u8DeckeyCb = nullptr;
        void *mMDkeyCbUserData = nullptr;
    };
}


#endif //FRAMEWORK_PLAYLIST_DEMUXER_H
