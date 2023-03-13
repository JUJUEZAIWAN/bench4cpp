#ifndef STONE_BUFFER_H
#define STONE_BUFFER_H


#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>

namespace stone
{
    using std::string;
    class Buffer 
    {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + initialSize),
              readerIndex_(kCheapPrepend),
              writerIndex_(kCheapPrepend)
        {
            assert(readableBytes() == 0);
            assert(writableBytes() == initialSize);
            assert(prependableBytes() == kCheapPrepend);
        }

  

        void swap(Buffer &rhs)
        {
            buffer_.swap(rhs.buffer_);
            std::swap(readerIndex_, rhs.readerIndex_);
            std::swap(writerIndex_, rhs.writerIndex_);
        }

        size_t readableBytes() const
        {
            return writerIndex_ - readerIndex_;
        }

        size_t writableBytes() const
        {
            return buffer_.size() - writerIndex_;
        }

        size_t prependableBytes() const
        {
            return readerIndex_;
        }

        const char *peek() const
        {
            return begin() + readerIndex_;
        }

        const char *findCRLF() const
        {
            // FIXME: replace with memmem()?
            const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? NULL : crlf;
        }

        const char *findCRLF(const char *start) const
        {
            assert(peek() <= start);
            assert(start <= beginWrite());
            // FIXME: replace with memmem()?
            const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? NULL : crlf;
        }

        const char *findEOL() const
        {
            const void *eol = memchr(peek(), '\n', readableBytes());
            return static_cast<const char *>(eol);
        }

        const char *findEOL(const char *start) const
        {
            assert(peek() <= start);
            assert(start <= beginWrite());
            const void *eol = memchr(start, '\n', beginWrite() - start);
            return static_cast<const char *>(eol);
        }

        // retrieve returns void, to prevent
        // string str(retrieve(readableBytes()), readableBytes());
        // the evaluation of two functions are unspecified
        void retrieve(size_t len)
        {
            assert(len <= readableBytes());
            if (len < readableBytes())
            {
                readerIndex_ += len;
            }
            else
            {
                retrieveAll();
            }
        }

        void retrieveUntil(const char *end)
        {
            assert(peek() <= end);
            assert(end <= beginWrite());
            retrieve(end - peek());
        }

        void retrieveInt64()
        {
            retrieve(sizeof(int64_t));
        }

        void retrieveInt32()
        {
            retrieve(sizeof(int32_t));
        }

        void retrieveInt16()
        {
            retrieve(sizeof(int16_t));
        }

        void retrieveInt8()
        {
            retrieve(sizeof(int8_t));
        }

        void retrieveAll()
        {
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend;
        }

        string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        string retrieveAsString(size_t len)
        {
            assert(len <= readableBytes());
            string result(peek(), len);
            retrieve(len);
            return result;
        }

        

        void append(const string &str)
        {
            append(str.data(), str.size());
        }

        void append(const char * /*restrict*/ data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }

        void append(const void * /*restrict*/ data, size_t len)
        {
            append(static_cast<const char *>(data), len);
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len)
            {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

        char *beginWrite()
        {
            return begin() + writerIndex_;
        }

        const char *beginWrite() const
        {
            return begin() + writerIndex_;
        }

        void hasWritten(size_t len)
        {
            assert(len <= writableBytes());
            writerIndex_ += len;
        }

        void unwrite(size_t len)
        {
            assert(len <= readableBytes());
            writerIndex_ -= len;
        }

        
        void prepend(const void * /*restrict*/ data, size_t len)
        {
            assert(len <= prependableBytes());
            readerIndex_ -= len;
            const char *d = static_cast<const char *>(data);
            std::copy(d, d + len, begin() + readerIndex_);
        }


        size_t internalCapacity() const
        {
            return buffer_.capacity();
        }

        /// Read data directly into buffer.
        ///
        /// It may implement with readv(2)
        /// @return result of read(2), @c errno is saved
        ssize_t readFd(int fd, int *savedErrno);

    private:
        char *begin()
        {
            return &*buffer_.begin();
        }

        const char *begin() const
        {
            return &*buffer_.begin();
        }

        void makeSpace(size_t len)
        {
            if (writableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                // FIXME: move readable data
                buffer_.resize(writerIndex_ + len);
            }
            else
            {
                // move readable data to the front, make space inside buffer
                assert(kCheapPrepend < readerIndex_);
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                          begin() + writerIndex_,
                          begin() + kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
                assert(readable == readableBytes());
            }
        }

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;

        static const char kCRLF[];
    };

}

#endif