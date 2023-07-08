//
//  compare.hpp
//  Created by rick gessner on 4/30/21.
//

#ifndef compare_h
#define compare_h

#include <sstream>

//Equal ------------------------------------------
namespace ECE141 {
    template<typename T>
    static bool isEqual(const T &arg1, const T &arg2) {
        return arg1 == arg2;
    }

    static bool isEqual(const std::string &aStr1, const std::string &aStr2) {
        return aStr1 == aStr2;
    }

    template<typename T1>
    static bool isEqual(const T1 &arg1, const std::string &aStr) {
        std::stringstream temp;
        temp << arg1;
        std::string theStr = temp.str();
        bool theResult = theStr == aStr;
        return theResult;
    }

    template<typename T1>
    static bool isEqual(const std::string &aStr, const T1 &arg2) {
        return isEqual(arg2, aStr);
    }

    template<typename T1, typename T2>
    static bool isEqual(const T1 &arg1, const T2 &arg2) {
        return static_cast<T1>(arg2) == arg1;
    }


    // Less than ------------------------------------------
    //same types
    template<typename T>
    static bool isLessThan(const T &arg1, const T arg2) {
        return arg1 < arg2;
    }

    // two strings
    static bool isLessThan(const std::string &aStr1, const std::string &aStr2) {
        return aStr1 < aStr2;
    }

    // two and static cast
    template<typename T1, typename T2>
    static bool isLessThan(const T1 &arg1, const T2 &arg2) {
        return static_cast<T1>(arg2) < arg1;
    }

    // template first arg, 2nd arg is str
    template<typename T1>
    static bool isLessThan(const T1 &arg1, const std::string &aStr) {
        std::stringstream temp;
        temp << arg1;
        std::string theStr = temp.str();
        bool theResult = theStr < aStr;
        return theResult;
    }

    // str, template
    template<typename T1>
    static bool isLessThan(const std::string &aStr, const T1 &arg1) {
        return isLessThan(arg1, aStr);
    }

    //    // string & bool
    //  bool isLessThan(const std::string &aStr, const bool &arg2) {
    //    return isLessThan(arg2,aStr);
    //}
    //    // string & int
    //  bool isLessThan(const std::string &aStr, const int &arg2) {
    //    return isLessThan(arg2,aStr);
    //}
    //    // string & double
    //  bool isLessThan(const std::string &aStr, const double &arg2) {
    //    return isLessThan(arg2,aStr);
    //}

    // Greater than ------------------------------------------
    // same type
    template<typename T>
    static bool isGreaterThan(const T &arg1, const T &arg2) {
        return arg1 > arg2;
    }

    // two strings
    static bool isGreaterThan(const std::string &aStr1, const std::string &aStr2) {
        return aStr1 > aStr2;
    }

    // static cast
    template<typename T1, typename T2>
    static bool isGreaterThan(const T1 &arg1, const T2 &arg2) {
        return static_cast<T1>(arg2) > arg1;
    }

    // arg1: template, arg2: str
    template<typename T1>
    static bool isGreaterThan(const T1 &arg1, const std::string &aStr) {
        return !(isLessThan(arg1, aStr) || isEqual(arg1, aStr));
    }

    // arg1: str, arg2:template
    template<typename T1>
    static bool isGreaterThan(const std::string &aStr, const T1 &arg1) {
        return !(isLessThan(aStr, arg1) || isEqual(aStr, arg1));
    }

    // Greater than or Equal To------------------------------------------

    // same type
    template<typename T>
    static bool isGreaterThanEqual(const T &arg1, const T &arg2) {
        return arg1 >= arg2;
    }

    // two strings
    static bool isGreaterThanEqual(const std::string &aStr1, const std::string &aStr2) {
        return aStr1 >= aStr2;
    }

    // static cast
    template<typename T1, typename T2>
    static bool isGreaterThanEqual(const T1 &arg1, const T2 &arg2) {
        return static_cast<T1>(arg2) >= arg1;
    }

    // arg1: template, arg2: str
    template<typename T1>
    static bool isGreaterThanEqual(const T1 &arg1, const std::string &aStr) {
        return !(isLessThan(arg1, aStr));
    }

    // arg1: str, arg2:str
    template<typename T1>
    static bool isGreaterThanEqual(const std::string &aStr, const T1 &arg1) {
        return !(isLessThan(aStr, arg1));
    }

    // Less than or Equal To------------------------------------------

    // same type
    template<typename T>
    static bool isLessThanEqual(const T &arg1, const T &arg2) {
        return arg1 <= arg2;
    }

    // two strings
    static bool isLessThanEqual(const std::string &aStr1, const std::string &aStr2) {
        return aStr1 <= aStr2;
    }

    // static cast
    template<typename T1, typename T2>
    static bool isLessThanEqual(const T1 &arg1, const T2 &arg2) {
        return static_cast<T1>(arg2) <= arg1;
    }

    // arg1: template, arg2: str
    template<typename T1>
    static bool isLessThanEqual(const T1 &arg1, const std::string &aStr) {
        return !(isGreaterThan(arg1, aStr));
    }

    // arg1: str, arg2:template
    template<typename T1>
    static bool isLessThanEqual(const std::string &aStr, const T1 &arg1) {
        return !(isGreaterThan(aStr, arg1));
    }
}

#endif /* compare_h */
