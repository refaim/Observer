#ifndef FB2_PARSER_H_
#define FB2_PARSER_H_

#include <exception>
#include <memory>
#include <string>

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

namespace fb2
{
    class RuntimeError : public std::exception {};

    class FictionBookSaxHandler : public xercesc::DefaultHandler
    {
    public:
        FictionBookSaxHandler() {}
        ~FictionBookSaxHandler() {} // TODO
    };

    class FictionBookSaxParser
    {
    public:
        FictionBookSaxParser();
        ~FictionBookSaxParser();
        void Parse(std::wstring path);
    };
}

#endif // FB2_PARSER_H_
