/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "number_identity_pinyin_helper.h"
#include "number_identity_log_wrapper.h"
#include "string_ex.h"

#include "unicode/utypes.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {

PinyinHelper::PinyinHelper() : transliterator(nullptr)
{
    UErrorCode status = U_ZERO_ERROR;
    auto transliterator = icu::Transliterator::createInstance(
        icu::UnicodeString("Han-Latin/Names; Latin-Ascii"), UTransDirection::UTRANS_FORWARD, status);
    if (transliterator == nullptr) {
        NUMBER_IDENTITY_LOGF("Failed to initialize icu::Transliterator, status: %{public}d", status);
        return;
    }
    NUMBER_IDENTITY_LOGD("icu::Transliterator init success.");
    this->transliterator = transliterator;
}

PinyinHelper::~PinyinHelper() {}

string PinyinHelper::GetPinyinAbbreviation(const string &pinyin)
{
    using namespace std;
    stringstream reader;
    stringstream writer;
    reader << pinyin;
    string buffer;
    while (reader >> buffer) {
        writer << buffer[0];
    }
    return writer.str();
}

string PinyinHelper::GetPinyinSubString(const string &pinyin)
{
    using namespace std;
    stringstream reader;
    stringstream writer;
    string buffer;
    reader << pinyin;
    vector<string> tokens;
    while (reader >> buffer) {
        tokens.emplace_back(buffer);
    }
    size_t words = tokens.size();
    for (size_t count = 1; count <= words; ++count) {
        writer << ' ';
        for (size_t offset = count; offset >= 1; --offset) {
            writer << tokens[words - offset];
        }
    }
    return writer.str();
}

string PinyinHelper::ConvertChineseToPinyin(const string &chinese)
{
    using icu::UnicodeString;
    if (transliterator == nullptr) {
        return "";
    }
    std::stringstream output;
    UnicodeString unicodeString(chinese.c_str());

    for (int i = 0, length = unicodeString.length(); i < length; ++i) {
        // The logic of double framework repo:
        // Transliterate Chinese pinyin character by character
        // to ensure every word is seperated.
        // e.g. pu fa yin hang 2 4 xiao shi ...
        //                     ^^^
        UChar32 unicodeChar = unicodeString.char32At(i);
        UnicodeString unicodeCharStr(unicodeChar);

        UnicodeString unicodeCharPinyin(unicodeCharStr);
        transliterator->transliterate(unicodeCharPinyin);

        string utf8CharPinyin;
        unicodeCharPinyin.toUTF8String(utf8CharPinyin);
        // Some Chinese words with multi pinyin transliterated
        // as single character is incorrect in the whole name.
        this->ForceMultiPinyinAsDefined(unicodeChar, utf8CharPinyin);
        output << " " << utf8CharPinyin;
    }

    return output.str();
}

map<UChar32, string> PinyinHelper::GetYellowPageMultiPinyins()
{
    NUMBER_IDENTITY_LOGD("GetYellowPageMultiPinyins");
    // In code of double framework repo, they are hard encoded.
    return {
        {0x8983, "QIN"},  {0x6c88, "SHEN"},  {0x66fe, "CENG"}, {0x8d3e, "JIA"},   {0x4fde, "YU"},   {0x513F, "ER"},
        {0x5475, "HE"},   {0x957f, "CHANG"}, {0x7565, "LUE"},  {0x63a0, "LUE"},   {0x4e7e, "QIAN"}, {0x79d8, "MI"},
        {0x8584, "bo"},   {0x79cd, "ZHONG"}, {0x891a, "chu"},  {0x555c, "chuo"},  {0x53e5, "ju"},   {0x839e, "guan"},
        {0x7094, "que"},  {0x85c9, "ji"},    {0x5708, "juan"}, {0x89d2, "jiao"},  {0x961a, "kan"},  {0x9646, "lu"},
        {0x7f2a, "miao"}, {0x4f74, "nai"},   {0x5152, "ni"},   {0x4e5c, "nie"},   {0x533a, "qu"},   {0x6734, "pu"},
        {0x7e41, "fan"},  {0x4ec7, "chou"},  {0x5355, "dan"},  {0x76db, "sheng"}, {0x6298, "zhe"},  {0x5bbf, "su"},
        {0x6d17, "xi"},   {0x89e3, "jie"},   {0x5458, "yuan"}, {0x7b2e, "ze"},    {0x7fdf, "zhai"}, {0x796d, "ji"},
        {0x963f, "a"},    {0x5b93, "mi"},    {0x90a3, "na"},   {0x5c09, "wei"},   {0x86fe, "e"},    {0x67e5, "cha"},
        {0x5200, "dao"},  {0x884c, "hang"},  {0x4e50, "le"},   {0x5730, "di"},    {0x5973, "nv"},   {0x8c03, "tiao"},
        {0x52d2, "le"},   {0x90fd, "du"},    {0x53a6, "xia"},
    };
}

void PinyinHelper::ForceMultiPinyinAsDefined(UChar32 chinese, string &pinyin)
{
    static auto multiPinyins = this->GetYellowPageMultiPinyins();
    auto entry = multiPinyins.find(chinese);
    if (entry == multiPinyins.end() || entry->second == pinyin) {
        return;
    }
    pinyin = entry->second;
}

} // namespace Telephony
} // namespace OHOS
