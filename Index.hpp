//
//  Index.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/2/21.
//

#ifndef Index_hpp
#define Index_hpp

#include <stdio.h>
#include <map>
#include <functional>
#include "Storage.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"


namespace ECE141 {
    using IndexVisitor = std::function<bool(const IndexKey &, uint32_t)>;

    struct Index : public Storable, BlockIterator {
        Index(std::string &aName, Storage &aStorage, uint32_t aBlockNum = 0,
              IndexType aType = IndexType::intKey)
                : name(aName), storage(aStorage), type(aType), blockNum(aBlockNum) {
            changed = false;
            tableId = 0;
        }

        class ValueProxy {
        public:
            Index &index;
            IndexKey key;
            IndexType type;

            ValueProxy(Index &anIndex, uint32_t aKey)
                    : index(anIndex), key(aKey), type(IndexType::intKey) {}

            ValueProxy(Index &anIndex, const std::string &aKey)
                    : index(anIndex), key(aKey), type(IndexType::strKey) {}

            ValueProxy &operator=(uint32_t aValue) {
                this->index.setKeyValue(key, aValue);

                return *this;
            }

            operator std::vector<uint32_t >() { return this->index.valueAt(key); }
        }; //value proxy

        ValueProxy operator[](const std::string &aKey) {
            return ValueProxy(*this, aKey);
        }

        ValueProxy operator[](uint32_t aKey) {
            return ValueProxy(*this, aKey);
        }

        uint32_t getBlockNum() const { return this->blockNum; }

        Index &setBlockNum(uint32_t aBlockNum) {
            this->blockNum = aBlockNum;
            return *this;
        }

        bool isChanged() { return this->changed; }

        Index &setChanged(bool aChanged) {
            this->changed = aChanged;

            return *this;
        }

        StorageInfo getStorageInfo(size_t aSize) {
            return StorageInfo{this->tableId, aSize,
                               static_cast<int32_t>(this->blockNum), BlockType::index_block};
        }

        IntList valueAt(IndexKey &aKey) {
            return exists(aKey) ? this->data[aKey] : IntList{};
        }

        IndexKey keyAt(IntOpt &aValue) {
            IndexKey theKey;

            this->eachKV([&](const IndexKey &aKey, const uint32_t &aBlockNum) {
                if(aBlockNum == *aValue) {
                    theKey = aKey;

                    return false;
                }

                return true;
            });

            return theKey;
        }

        bool setKeyValue(IndexKey &aKey, uint32_t aValue) {
            this->data[aKey].push_back(aValue);

            return this->changed = true; //side-effect indended!
        }

        StatusResult erase(const std::string &aKey) {
            if (this->data.count(aKey)) {
                this->data.erase(aKey);
            }
            return StatusResult{Errors::noError};
        }

        StatusResult erase(uint32_t aKey) {
            return StatusResult{Errors::noError};
        }

        size_t getSize() { return this->data.size(); }

        bool exists(IndexKey &aKey) {
            return this->data.count(aKey);
        }

        StatusResult encode(std::ostream &anOutput) override {
            std::string tempName = this->name;
            std::replace(tempName.begin(), tempName.end(), ' ', '~');
            anOutput << tempName << ' ' << this->blockNum << ' ' <<
            this->tableId << ' ' << this->data.size() << ' ';
            
            for (auto& thePair : this->data) {
                switch (thePair.first.index()) {
                    case 0:
                        anOutput << "i " << thePair.second.size() << " "
                        << std::get<uint32_t>(thePair.first) << " ";
                        break;
                    case 1:
                    default:
                        anOutput << "s " << thePair.second.size() << " "
                        << std::get<std::string>(thePair.first) << " ";
                }

                for(auto& val: thePair.second) {
                    anOutput << val << " ";
                }
            }

            return StatusResult{Errors::noError};
        }

        StatusResult decode(std::istream &anInput) override {
            std::string tempName;
            anInput >> tempName >> this->blockNum >> this->tableId;
            std::replace(tempName.begin(), tempName.end(), '~', ' ');
            this->name = tempName;


            size_t theCount;
            anInput >> theCount;

            for (size_t i = 0; i < theCount; i++) {
                decodeValues(anInput);
            }

            return StatusResult{Errors::noError};
        }

        void decodeValues(std::istream &anInput) {
            char theType;
            uint32_t theValue;
            size_t theAttrCount;
            IndexKey theKey;

            anInput >> theType >> theAttrCount;

            decodeType(anInput, theKey, theType);

            for(size_t i = 0; i < theAttrCount; i++) {
                anInput >> theValue;

                this->data[theKey].push_back(theValue);
            }
        }

        void decodeType(std::istream &anInput, IndexKey &aKey, char aType) {
            std::string theSKey;
            uint32_t theIKey;

            switch (aType) {
                case 'i':
                    anInput >> theIKey;
                    aKey = theIKey;
                    break;
                case 's':
                default:
                    anInput >> theSKey;
                    aKey = theSKey;
            }
        }

        bool each(const BlockVisitor &aVisitor) override {
            Block theBlock;

            for (auto thePair : this->data) {
                for(auto thePos: thePair.second) {
                    if (this->storage.readBlock(thePos, theBlock)) {
                        if (!aVisitor(theBlock, thePos)) { return false; }
                    }
                }
            }

            return true;
        }

        bool eachKV(IndexVisitor aCall) {
            for (auto thePair : this->data) {
                for(auto thePos: thePair.second) {
                    if (!aCall(thePair.first, thePos)) {
                        return false;
                    }
                }
            }

            return true;
        }

        std::string getName() {
            return this->name;
        }

        void setTableId(std::string aTableName) {
            this->tableId = Entity::hashString(aTableName);
        }

    protected:

        Storage &storage;
        std::map<IndexKey, IntList> data;
        IndexType type;
        std::string name;
        bool changed;
        uint32_t blockNum;
        uint32_t tableId;
    };
}


#endif /* Index_hpp */
