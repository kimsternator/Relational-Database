//
//  Storage.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "Storage.hpp"
#include "Entity.hpp"
#include "Config.hpp"
#include "Row.hpp"

namespace ECE141 {


    // USE: ctor ---------------------------------------
    Storage::Storage(std::iostream &aStream) : BlockIO(aStream) {
    }

    // USE: dtor ---------------------------------------
    Storage::~Storage() {
    }

    bool Storage::each(const BlockVisitor &aVisitor) {
        uint32_t count = getBlockCount();

        for (uint32_t i = 0; i < count; i++) {
            Block theBlock;
            readBlock(i, theBlock);
            if (!aVisitor(theBlock, i)) break;
        }
        return true;
    }

    void Storage::prepareBlock(Block &aBlock, StorageInfo &anInfo) {
        aBlock.header.refId = anInfo.refId;
        std::memset(aBlock.payload, ' ', actualKPayloadSize);
        aBlock.payload[actualKPayloadSize] = '\0';
    }

    void Storage::setNextBlock(Block &aBlock, uint32_t &aNextBlockNum) {
        aNextBlockNum = peekNextFreeBlock();
        aBlock.header.next = (aNextBlockNum == aBlock.header.pos) ?
                             aNextBlockNum + 1 : aNextBlockNum;
        aNextBlockNum = aBlock.header.next;
    }

    //write logic to get the next available free block
    uint32_t Storage::getFreeBlock() {
        if (this->available.empty()) {
            return getBlockCount();
        }

        uint32_t thePos = this->available.top();
        this->available.pop();

        return thePos;
    }

    uint32_t Storage::peekNextFreeBlock() {
        if (this->available.empty()) {
            return getBlockCount();
        }

        return this->available.top();
    }

    bool Storage::blockAlreadyInUse(uint32_t aBlockNum) {
        if(this->available.empty() && aBlockNum < peekNextFreeBlock()) {
            return true;
        }
        else if(aBlockNum >= peekNextFreeBlock()) {
            return false;
        }

        std::vector<uint32_t> tempPQ;
        bool done = false;

        while(!done) {
            if(this->available.empty() || aBlockNum < this->available.top()) {
                returnValues(tempPQ);

                return false;
            }

            if(this->available.top() == aBlockNum) {
                returnValues(tempPQ);

                return true;
            }

            tempPQ.push_back(this->available.top());
            this->available.pop();
        }

        return false;
    }

    void Storage::returnValues(std::vector<uint32_t> &theBorrowed) {
        for(auto val: theBorrowed) {
            this->available.push(val);
        }
    }

    //write logic to mark a block as free...
    StatusResult Storage::markBlockAsFree(uint32_t aPos) {
        StatusResult theResult;
        Block theBlock;

        if ((theResult = readBlock(aPos, theBlock))) {
            theBlock.header.type = (char) BlockType::free_block;
        }

        theResult = writeBlock(aPos, theBlock);
        this->available.push(aPos);

        return theResult;
    }

    // USE: for use with storable API...
    //   Write logic to mark a sequence of blocks as free)
    //   starting at given block number, following block sequence
    //   defined in the block headers...
    StatusResult Storage::releaseBlocks(uint32_t aPos, bool aInclusive) {
        return StatusResult{Errors::noError};
    }

    //Write logic to break stream into N parts, that fit into
    //a series of blocks. Save each block, and link them together
    //logically using data in the header...
    //save a sequence of new block
    StatusResult Storage::save(std::iostream &aStream, StorageInfo &anInfo) {
        size_t theSize = anInfo.size;
        //block actually in use so updating
        if(blockAlreadyInUse(anInfo.start)) {
            return update(aStream, anInfo);
        }

        //save a single block
        if (aStream && theSize > 0) {
            Block theBlock(anInfo.type);
            theBlock.header.pos = anInfo.start;
            prepareBlock(theBlock, anInfo);
            aStream.read(theBlock.payload, std::min(theSize, actualKPayloadSize));
            theSize -= std::min(theSize, actualKPayloadSize);
            uint32_t theNextBlockNum = 0;

            if (theSize > 0) {
                setNextBlock(theBlock, theNextBlockNum);
            }

            writeBlock(theBlock.header.pos, theBlock);

            //save more blocks in a sequence
            while (aStream && theSize > 0) {
                Block theNextBlock(anInfo.type);
                theNextBlock.header.pos = getFreeBlock();
                prepareBlock(theNextBlock, anInfo);
                aStream.read(theNextBlock.payload, std::min(theSize, actualKPayloadSize));
                theSize -= std::min(theSize, actualKPayloadSize);

                if (theSize > 0) {
                    setNextBlock(theNextBlock, theNextBlockNum);
                }

                writeBlock(theNextBlock.header.pos, theNextBlock);
            }

            return StatusResult();
        }

        return StatusResult{Errors::writeError};
    }

    //update an existing block
    StatusResult Storage::update(std::iostream &aStream, StorageInfo &anInfo) {
        size_t theSize = anInfo.size;
        //save a single block
        if (aStream && theSize > 0) {
            Block theBlock;
            StatusResult theResult = readBlock(anInfo.start, theBlock);
            uint32_t theNextBlockNum = 0;

            if (theResult == StatusResult()) {
                theBlock.header.pos = anInfo.start;
                prepareBlock(theBlock, anInfo);
                aStream.read(theBlock.payload, std::min(theSize, actualKPayloadSize));
                theSize -= std::min(theSize, actualKPayloadSize);

                if (theSize > 0) {
                    if (theBlock.header.next == 0) {
                        StorageInfo theNewInfo = anInfo;
                        theNewInfo.size = theSize;
                        theNewInfo.start = getFreeBlock();
                        theBlock.header.next = theNewInfo.start;
                        writeBlock(theBlock.header.pos, theBlock);

                        return save(aStream, theNewInfo);
                    }

                    theNextBlockNum = theBlock.header.next;
                }

                writeBlock(theBlock.header.pos, theBlock);

                while (aStream && theSize > 0) {
                    Block theNextBlock;
                    theResult = readBlock(theNextBlockNum, theNextBlock);

                    if (theResult) {
                        prepareBlock(theNextBlock, anInfo);
                        aStream.read(theNextBlock.payload, std::min(theSize, actualKPayloadSize));
                        theSize -= std::min(theSize, actualKPayloadSize);

                        if (theSize > 0) {
                            if (theNextBlock.header.next == 0) {
                                StorageInfo theNewInfo = anInfo;
                                theNewInfo.size = theSize;
                                theNewInfo.start = getFreeBlock();
                                theNextBlock.header.next = theNewInfo.start;
                                writeBlock(theNextBlock.header.pos, theNextBlock);

                                return save(aStream, theNewInfo);
                            }

                            theNextBlockNum = theNextBlock.header.next;
                        }

                        writeBlock(theNextBlock.header.pos, theNextBlock);
                    } else {
                        return StatusResult{Errors::readError};
                    }
                }
            } else {
                return StatusResult{Errors::readError};
            }
        }

        return StatusResult();
    }

    //Write logic to read an ordered sequence of N blocks, back into
    //a stream for your caller
    StatusResult Storage::load(std::iostream &anOut, uint32_t aBlockNum) {
        Block theBlock;
        StatusResult theResult = readBlock(aBlockNum, theBlock);

        if (theResult == StatusResult()) {
            anOut.write(theBlock.payload, std::strlen(theBlock.payload));

            while (theResult == StatusResult() && theBlock.header.next != 0) {
                theResult = readBlock(theBlock.header.next, theBlock);

                if (theResult == StatusResult()) {
                    anOut.write(theBlock.payload, std::strlen(theBlock.payload));
                }
            }
        }

        return theResult;
    }

}

