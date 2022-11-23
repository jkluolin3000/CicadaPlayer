//
// Created by moqi on 2018/4/27.
//

#ifndef FRAMEWORK_ABSTRACTSTREAM_H
#define FRAMEWORK_ABSTRACTSTREAM_H

/*
 * AbstractStream is a single stream proved form playList file,
 * maybe a ts muxed with one or more es stream,or a aac file
 */
#include "SegmentPart.h"
#include "utils/AFMediaType.h"
#include <base/OptionOwner.h>
#include <demuxer/demuxer_service.h>

namespace Cicada {

    class AbstractStream : public OptionOwner {
    public:
        AbstractStream();

        virtual ~AbstractStream();

        virtual int open() = 0;

        virtual void close() = 0;

        virtual int read(std::unique_ptr<IAFPacket> &packet) = 0;

        virtual int GetNbStreams() const = 0;

        virtual int GetStreamMeta(Stream_meta *meta, int index, bool sub) const = 0;

        virtual bool CloseSubStream(int index)
        {
            return false;
        }

        virtual bool isOpened() = 0;

        virtual int start() = 0;

        virtual int preStop() = 0;

        virtual int stop() = 0;

        virtual int64_t seek(int64_t us, int flags) = 0;

        virtual uint64_t getCurSegNum() = 0;

        virtual int stopOnSegEnd(bool stop) = 0;

        class CurSegInfo {
        public:
            uint64_t segNum{0};
            uint64_t position{0};
        };

        virtual int setCurSegInfo(CurSegInfo &curSegInfo)
        {
            return 0;
        };

        virtual void setCurRenditionInfo(const std::vector<RenditionReport> &renditions)
        {}

        virtual std::vector<RenditionReport> getCurRenditionInfo()
        {
            return {};
        }

        virtual int SetCurSegNum(uint64_t num) = 0;

        virtual uint64_t getCurSegPosition() = 0;

        virtual int setCurSegPosition(uint64_t position) = 0;

        virtual bool isLive() = 0;

        virtual int64_t getDuration() = 0;

        virtual int getNBStream() const = 0;

        virtual void interrupt(int inter) = 0;

        virtual void setExtDataSource(IDataSource *source)
        {
            mExtDataSource = source;
        }


        virtual void setDataSourceConfig(const IDataSource::SourceConfig &config)
        {
            mSourceConfig = config;
        }

        virtual void setBitStreamFormat(header_type vMergeHeader, header_type aMergeHeader)
        {
            mMergeVideoHeader = vMergeHeader;
            mMergerAudioHeader = aMergeHeader;
        }

        virtual void setUrlToUniqueIdCallback(UrlHashCB cb, void *userData)
        {
            mUrlHashCb = cb;
            mUrlHashCbUserData = userData;
        }
        virtual void setM3u8DecryptKeyCallBack(m3u8_decrypt_key_callback m3u8DeckeyCb, void *userData) {
            mM3u8DeckeyCb = m3u8DeckeyCb;
            mMDkeyCbUserData = userData;
        }

    protected:
        IDataSource *mExtDataSource = nullptr;
        IDataSource::SourceConfig mSourceConfig{};
        header_type mMergeVideoHeader = header_type::header_type_no_touch;
        header_type mMergerAudioHeader = header_type::header_type_no_touch;
        UrlHashCB mUrlHashCb{nullptr};
        void *mUrlHashCbUserData{nullptr};
        //
        m3u8_decrypt_key_callback mM3u8DeckeyCb = nullptr;
        void *mMDkeyCbUserData = nullptr;
    };
}// namespace Cicada


#endif//FRAMEWORK_ABSTRACTSTREAM_H
