/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: fieldvalue.cxx,v 1.3 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "StdAfx.h"
#include <mimetic/rfc822/fieldvalue.h>

namespace mimetic
{
using namespace std;

std::ostream& operator<<(std::ostream& os, const FieldValue& fv)
{
    return os << fv.str();
}

FieldValue::FieldValue()
:m_typeChecked(true) // true for all class that don't handle this flag
{
}

FieldValue::~FieldValue()
{
}

bool FieldValue::typeChecked() const
{
    return m_typeChecked;
}

void FieldValue::typeChecked(bool b)
{
    m_typeChecked = b;
}

// StringFieldValue
StringFieldValue::StringFieldValue()
{
    typeChecked(false);
}

StringFieldValue::StringFieldValue(const string& val)
: m_value(val)
{
    typeChecked(false);
}

void StringFieldValue::set(const string& val)
{
    m_value = val;
}

std::string StringFieldValue::str() const
{
    return m_value;
}

const std::string& StringFieldValue::ref() const
{
    return m_value;
}

std::string& StringFieldValue::ref()
{
    return m_value;
}

FieldValue* StringFieldValue::clone() const
{
    return new StringFieldValue(*this);
}


}
