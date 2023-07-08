//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <iomanip>

#include "Storage.hpp"
#include "keywords.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {

  class Attribute : public Storable {
  protected:
    std::string             name;
    DataTypes               type;
    std::optional<size_t>   fieldLength;
    bool                    auto_increment;
    bool                    primary_key;
    bool                    nullable;
    
    //what other data members do attributes required?
    
  public:
          
      //STUDENT: declare ocf methods...
      Attribute() {
          type = DataTypes::no_type;
          auto_increment = false;
          primary_key = false;
          nullable = true;
      }
    
      //What methods do you need to interact with Attributes?
      StatusResult initialize(std::string &aName, DataTypes aType,
                              std::optional<size_t> aLen, bool aAuto_increment=false,
                              bool aPrimary_key=false, bool aNullable=true);

      void describeDisplay(std::ostream &anOutput);

      static std::string typeToString(DataTypes aType);
      static DataTypes stringToType(std::string &aType);
      static DataTypes indexToType(size_t anIndex);
      std::string defaultVal();
      Value defaultValueVal();
      Value nullValueVal();

      StatusResult setPrimary();

      bool isNullable();
      bool isAuto();
      bool isPrimary();
      StatusResult typeMatch(size_t anIndex);

      std::string& getName();
      DataTypes getType();

      //Added so that the attribute is a storable...
    StatusResult        encode(std::ostream &aWriter);
    StatusResult        decode(std::istream &aReader);
    
  };
  
}


#endif /* Attribute_hpp */
