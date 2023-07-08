//
//  Storage.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <functional>
#include <queue>
#include "BlockIO.hpp"
#include "Errors.hpp"

namespace ECE141 {

    class Entity;

    class Row;

    struct CreateDB {
    }; //tags for db-open modes...
    struct OpenDB {
    };

    const int32_t kNewBlock = -1;

    class Storable {
    public:
        virtual StatusResult encode(std::ostream &anOutput) = 0;

        virtual StatusResult decode(std::istream &anInput) = 0;
    };

    struct StorageInfo {

        StorageInfo(size_t aRefId, size_t theSize, int32_t aStartPos = kNewBlock,
                    BlockType aType = BlockType::data_block)
                : type(aType), start(aStartPos), refId(aRefId), size(theSize) {}

        BlockType type;
        int32_t start; //block#
        size_t refId;
        size_t size;
    };

    using BlockList = std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>>;
    using BlockVisitor = std::function<bool(const Block&, uint32_t)>;

    struct BlockIterator {
        virtual bool each(const BlockVisitor &aVisitor)=0;
    };

    // USE: Our storage class (for stream IO)
    class Storage : public BlockIO, public BlockIterator {
    public:

        Storage(std::iostream &aStream);

        ~Storage();

        bool each(const BlockVisitor &aVisitor);

        StatusResult markBlockAsFree(uint32_t aPos);

        void prepareBlock(Block &aBlock, StorageInfo &anInfo);
        void setNextBlock(Block &aBlock, uint32_t &aNextBlockNum);

        uint32_t getFreeBlock(); //pos of next free (or new)...
        uint32_t peekNextFreeBlock();
        bool blockAlreadyInUse(uint32_t aBlockNum);
        void returnValues(std::vector<uint32_t> &theBorrowed);

        StatusResult save(std::iostream &aStream, StorageInfo &anInfo);
        StatusResult update(std::iostream &aStream, StorageInfo &anInfo);
        StatusResult load(std::iostream &aStream, uint32_t aStartBlockNum);

    protected:
        StatusResult releaseBlocks(uint32_t aPos, bool aInclusive = false);

        BlockList available;
    };

}


#endif /* Storage_hpp */
