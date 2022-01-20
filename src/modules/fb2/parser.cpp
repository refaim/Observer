// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "parser.h"

#include <memory>
#include <string>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLUni.hpp>

namespace fb2
{
    FictionBookSaxParser::FictionBookSaxParser()
    {
        try
        {
            xercesc::XMLPlatformUtils::Initialize();
        }
        catch (xercesc::XMLException&)
        {
            throw RuntimeError();
        }
    }

    FictionBookSaxParser::~FictionBookSaxParser()
    {
        try
        {
            xercesc::XMLPlatformUtils::Terminate();
        }
        catch (...) {}
    }

    void FictionBookSaxParser::Parse(std::wstring path)
    {
        auto handler = std::make_unique<FictionBookSaxHandler>();
        auto reader = std::unique_ptr<xercesc::SAX2XMLReader>(xercesc::XMLReaderFactory::createXMLReader());
        // TODO somehow enable offsets calc
        reader->setFeature(xercesc::XMLUni::fgSAX2CoreValidation, false);
        /*
    http://xml.org/sax/features/validation (default: true)
    http://xml.org/sax/features/namespaces (default: true)
    http://xml.org/sax/features/namespace-prefixes (default: false)
    http://apache.org/xml/features/validation/dynamic (default: false)
    http://apache.org/xml/features/validation/reuse-grammar (default: false)
    http://apache.org/xml/features/validation/schema (default: true)
    http://apache.org/xml/features/validation/schema-full-checking (default: false)
    http://apache.org/xml/features/validating/load-schema (default: true)
    http://apache.org/xml/features/nonvalidating/load-external-dtd (default: true)
    http://apache.org/xml/features/continue-after-fatal-error (default: false)
    http://apache.org/xml/features/validation-error-as-fatal (default: false)
    */
        reader->setContentHandler(handler.get());
        reader->setErrorHandler(handler.get());

        try
        {
            // TODO test cyrillic filename
            // WideCharToMultiByte(CP_UTF8);
            // reader_->parse(path.c_str());
        }
        catch (xercesc::XMLException&)
        {
            throw RuntimeError();
        }
        catch (xercesc::SAXParseException&)
        {
            throw RuntimeError();
        }
    }
}
