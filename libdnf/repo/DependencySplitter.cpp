/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "DependencySplitter.hpp"
#include "../dnf-sack.h"
#include "../utils/regex/regex.hpp"


static const Regex RELDEP_REGEX = 
    Regex("^([^ \t\r\n\v\f<=>!]*)\\s*(<=|>=|!=|<|>|=)?\\s*(.*)$", REG_EXTENDED);

static bool
getCmpFlags(int *cmp_type, std::string matchCmpType)
{
    int subexpr_len = matchCmpType.size();
    auto match_start = matchCmpType.c_str();
    if (subexpr_len == 2) {
        if (strncmp(match_start, "!=", 2) == 0)
            *cmp_type |= HY_NEQ;
        else if (strncmp(match_start, "<=", 2) == 0) {
            *cmp_type |= HY_LT;
            *cmp_type |= HY_EQ;
        }
        else if (strncmp(match_start, ">=", 2) == 0) {
            *cmp_type |= HY_GT;
            *cmp_type |= HY_EQ;
        }
        else
            return false;
    } else if (subexpr_len == 1) {
        if (*match_start == '<')
            *cmp_type |= HY_LT;
        else if (*match_start == '>')
            *cmp_type |= HY_GT;
        else if (*match_start == '=')
            *cmp_type |= HY_EQ;
        else
            return false;
    } else
        return false;
    return true;
}

namespace libdnf {

bool
DependencySplitter::parse(const char * reldepStr)
{
    enum { NAME = 1, CMP_TYPE = 2, EVR = 3, _LAST_ };
    auto matchResult = RELDEP_REGEX.match(reldepStr, false, _LAST_);
    if (!matchResult.isMatched() || matchResult.getMatchedLen(NAME) == 0)
         return false;
    name = matchResult.getMatchedString(NAME);
    evr = matchResult.getMatchedString(EVR);
    cmpType = 0;
    if (!matchResult.getMatchedLen(EVR) && !matchResult.getMatchedLen(CMP_TYPE))
        return true;
    if (!(matchResult.getMatchedLen(EVR) && matchResult.getMatchedLen(CMP_TYPE)))
        return false;

    return getCmpFlags(&cmpType, matchResult.getMatchedString(CMP_TYPE));
}

}