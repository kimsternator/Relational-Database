//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderView_h
#define FolderView_h

#include "View.hpp"
#include "FolderIterator.hpp"
#include <cmath>

namespace ECE141 {

  class FolderView : public View {
  public:
    FolderView(const char *aPath, const char *anExtension="db")
      : iter(aPath), extension(anExtension) {}
   
    virtual bool show(std::ostream &anOutput) override {
        std::string theBar = "+-----------------+\n";
        size_t count = 0;

        anOutput << theBar;
        anOutput << "+ Databases       +\n" << theBar;

        iter.each(extension, [&](const std::string &aName) {
            anOutput << "| " << std::left << std::setw(16) << aName << "|\n";
            anOutput << theBar;
            count++;
            return true;
        });


        anOutput << count << " rows in set ";

        return true;
    }

    FolderIterator iter; //need to create

    const char *path;
    const char *extension;
  };

}

#endif /* FolderView_h */
