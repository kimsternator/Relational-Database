//
// Created by kevin on 4/11/2021.
//

#ifndef ECE141B_FOLDERITERATOR_HPP
#define ECE141B_FOLDERITERATOR_HPP

#endif //ECE141B_FOLDERITERATOR_HPP

#include <string>
#include <filesystem>
#include <fstream>
#include <functional>

namespace fs = std::filesystem;

namespace ECE141 {
    using FileVisitor = std::function<bool(const std::string&)>;

    class FolderIterator {
        public:
            FolderIterator(const char *aPath) : path(aPath) {}
            virtual ~FolderIterator() {}
            virtual bool exists(const std::string &aFilename) {
                std::ifstream theStream(aFilename);
                return !theStream ? false : true;
            }
            virtual void each(const std::string &anExt, const FileVisitor &aVisitor) const{
                fs::path thePath(path);
                for (auto & theItem : fs::directory_iterator(path)) {
                    if (!theItem.is_directory()) {
                        fs::path temp(theItem.path());
                        std::string anExt = (temp.extension().string());
                        if (0==anExt.size() || 0==anExt.compare(anExt)) {
                            if(!aVisitor(temp.stem().string())) break;
                        }
                    }
                }
            }

            std::string path;
    };
}