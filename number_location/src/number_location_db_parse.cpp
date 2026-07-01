/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "number_location_db_parse.h"

#include <regex>
#include <fstream>
#include <string>
#include <cstring>

#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_location_parser.h"

namespace OHOS {
namespace Telephony {
const char *PATH = "/system/etc/telephony/numberlocation.dat";
const char *PATH_UPDATE = "/data/storage/el2/base/files/numberlocation.dat";
const char *NUMBER_LOCATION_DATA_PATH = "/system/etc/telephony/numberlocation.data";
const char *NUMBER_LOCATION_DATA_PATH_UPDATE = "/data/storage/el2/base/files/numberlocation.data";
const char *BLOCKMAPPSTART = "BlockMapping start:";
const char *BLOCKMAPPEND = "BlockMapping end";
const int32_t BLOCKMAP_ADDR_OFFSET = -(1024 + 20 + 20);
const int32_t BLOCKMAP_FLAG_LENGTH = 20;
const int32_t BLOCKMAP_DATA_LENGTH = 1024;
const int32_t MAXCITYNAME_UNICODE_BUF = 22;
const int32_t MAXCITYNAME_UTF8_BUF = 32;
const int32_t PREFIXFILESIZE = 200000;
const int32_t CONFIG_BUFFER_LENGTH = 1124;
const int32_t PREFIX_NUMBER_LIST = 100;
const int32_t PREFIX_NUMBER_LENGTH = 3;
const int32_t NUMBER_SEGMENT_LENGTH = 10;
const int32_t MAX_MOBILE_PHONE_NUMBER_LENGTH = 12;
const int32_t QUHAO_LENGTH_LIMIT = 10;
const int32_t NUMBER_LIMIT_MIN = 5;
const int32_t NUMBER_LIMIT_OPERATOR_NAME_PART = 7;
const int32_t MAX_MOBILE_OP_NAME_LEN = 40;
const int32_t NUMBER_LIMIT_CITY_PART = 4;
const int32_t PREFIX_TWO_LENGTH = 2;
const int32_t MAX_DIGIT_NUMBER = 9;
const int32_t MOVE_RIGHT_FOUR = 4;
const int32_t MOVE_RIGHT_SIX = 6;
const int32_t MAX_CITY_INDEX = 32;
const int32_t OPERATOR_NUMBER_LENGTH = 4;
const int32_t MAX_ASCII = 256;

NumberLocationDbParse::NumberLocationDbParse() : dataLoaded_(false) {}

NumberLocationDbParse::~NumberLocationDbParse() {}

char NumberLocationDbParse::GetEndNum(wchar_t *pText, int32_t len)
{
    for (int32_t i = len - 1; i >= 0; i--) {
        if (pText[i] != 0) {
            return (MAX_DIGIT_NUMBER - i + 0x30);
        }
    }
    return MAX_DIGIT_NUMBER + 0x30;
}

char NumberLocationDbParse::GetActualNum(wchar_t *pText, int32_t len)
{
    for (int32_t i = 0; i < len; i++) {
        if (pText[i] == 0) {
            return i + 0x30;
        }
    }
    return len + 0x30;
}

void NumberLocationDbParse::UnicodeToUTF_8(char *pOut, wchar_t *pText)
{
    char *pchar = (char *)pText;
    pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> MOVE_RIGHT_FOUR));
    pOut[1] = (0x80 | ((pchar[1] & 0x0F) << PREFIX_TWO_LENGTH)) + ((pchar[0] & 0xC0) >> MOVE_RIGHT_SIX);
    pOut[PREFIX_TWO_LENGTH] = (0x80 | (pchar[0] & 0x3F));
    return;
}

void NumberLocationDbParse::TransformUnicodeToUTF8(char *cityNameUnicode, char *cityNameUTF8, int32_t lenthUTF8)
{
    if (!strcmp(cityNameUnicode, " ")) {
        cityNameUTF8[0] = 0;
        return;
    }
    wchar_t ret1[NUMBER_SEGMENT_LENGTH];
    for (int32_t i = 0; i < NUMBER_SEGMENT_LENGTH; i++) {
        ret1[i] = cityNameUnicode[PREFIX_TWO_LENGTH * i + 1] * MAX_ASCII + cityNameUnicode[PREFIX_TWO_LENGTH * i];
    }
    char endNum = GetEndNum(ret1, NUMBER_SEGMENT_LENGTH);
    uint32_t realIntNum = NUMBER_SEGMENT_LENGTH - (endNum - '0');
    char realCharNum = realIntNum + '0';

    char actualCharNum = GetActualNum(ret1, 10);
    int32_t actualIntNum = actualCharNum - '0';
    cityNameUTF8[0] = realCharNum;
    cityNameUTF8[1] = actualCharNum;
    char *p1 = &cityNameUTF8[PREFIX_TWO_LENGTH];
    wchar_t *p2 = &ret1[0];
    for (int32_t i = 0; i < actualIntNum; i++) {
        if (*p2 == '/') {
            *p1 = '/';
            p1 = p1 + 1;
            p2 = p2 + 1;
            continue;
        }
        UnicodeToUTF_8(p1, p2);
        p1 = p1 + PREFIX_NUMBER_LENGTH;
        p2 = p2 + 1;
    }

    for (int32_t i = PREFIX_TWO_LENGTH + actualIntNum * PREFIX_NUMBER_LENGTH; i < MAX_CITY_INDEX; i++) {
        cityNameUTF8[i] = 0;
    }
}

FILE *NumberLocationDbParse::OpenDataFile()
{
    FILE *fp = fopen(PATH_UPDATE, "rb");
    if (fp == nullptr) {
        NUMBER_IDENTITY_LOGW("fopen PATH_UPDATE fail.");
        fp = fopen(PATH, "rb");
    }
    if (fp == nullptr) {
        NUMBER_IDENTITY_LOGE("open PATH_UPDATE & PATH fail, errno: %{public}d: %{public}s", errno, strerror(errno));
    }
    return fp;
}
// get unicode city name by city index.
void NumberLocationDbParse::GetCityName(
    FILE *fileHndRf, void *configbuffer, int32_t cityIndex, unsigned char *outCityName)
{
    uint32_t rsize = 0;
    int32_t pos = ReadBlockFilePosition(
        configbuffer, NUMBER_SEGMENT_LENGTH, static_cast<int32_t>(MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_CITYNAMEDATA));
    // 20 digits per city name.
    int32_t seekLength = pos + sizeof(unsigned char) * BLOCKMAP_FLAG_LENGTH * cityIndex;
    fseek(fileHndRf, seekLength, SEEK_SET);
    rsize = fread(outCityName, 1, sizeof(unsigned char) * BLOCKMAP_FLAG_LENGTH, fileHndRf);
    outCityName[rsize] = 0;
}

void NumberLocationDbParse::GetCityIndexByPhoneNum(
    FILE *fileHndRf, void *configbuffer, int32_t phonNumIndex, int32_t prefixIndex, void *rtData)
{
    int32_t pos = ReadBlockFilePosition(
        configbuffer, NUMBER_SEGMENT_LENGTH, static_cast<int32_t>(MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_PHONENUMLIST));
    int32_t seekLength = pos + sizeof(short) * phonNumIndex + prefixIndex * PREFIXFILESIZE;
    fseek(fileHndRf, seekLength, SEEK_SET);
    fread(rtData, 1, sizeof(short), fileHndRf);
}

void NumberLocationDbParse::ReadBlockById(void *data, int32_t blockId, BlockMapStr *outData)
{
    char *ptData = (char *)data;
    if (memcpy_s(
        outData, sizeof(BlockMapStr), ptData + sizeof(BlockMapStr) * blockId, sizeof(BlockMapStr)) != EOK) {
        NUMBER_IDENTITY_LOGE("readBlockById failed to memcpy_s");
        return; // return error code -1
    }
    return;
}

int32_t NumberLocationDbParse::ReadBlockFilePosition(void *configbuffer, int32_t bufferLenth, int32_t blockId)
{
    BlockMapStr blockMappStrTemp;
    NUMBER_IDENTITY_LOGD("ReadBlockFilePosition: configBuffer len = %{public}d", bufferLenth);
    ReadBlockById(configbuffer, blockId, &blockMappStrTemp);
    return blockMappStrTemp.blockFilePos;
}

// read phone number prefix list from buffer.
void NumberLocationDbParse::ReadPhoneNumberPrefixList(FILE *fp, void *configBuffer, int32_t bufferLenth, char *outData)
{
    BlockMapStr blockMappStrTemp;
    NUMBER_IDENTITY_LOGD("ReadPhoneNumberPrefixList: configBuffer len = %{public}d", bufferLenth);
    ReadBlockById(configBuffer, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_PHONENUMPREFIXLIST), &blockMappStrTemp);
    fseek(fp, blockMappStrTemp.blockFilePos, SEEK_SET);
    fread(outData, 1, blockMappStrTemp.block_size, fp);
}

int32_t NumberLocationDbParse::ReadBlockConfigToBuffer(FILE *fp, void *outData, int32_t len)
{
    fseek(fp, BLOCKMAP_ADDR_OFFSET, SEEK_END);
    if (memset_s(outData, len, 0, len) != EOK) {
        NUMBER_IDENTITY_LOGE("ReadBlockConfigToBuffer failed to memset_s");
        return NUMBER_IDENTITY_ERR_MEMSET_FAIL;
    }

    fread(outData, 1, BLOCKMAP_FLAG_LENGTH, fp);
    if (memcmp(outData, BLOCKMAPPSTART, strlen(BLOCKMAPPSTART)) == 0) {
        if (memset_s(outData, len, 0, len) != EOK) {
            NUMBER_IDENTITY_LOGE("ReadBlockConfigToBuffer failed to memset_s");
            return NUMBER_IDENTITY_ERR_MEMSET_FAIL;
        }

        fseek(fp, -(BLOCKMAP_FLAG_LENGTH), SEEK_END);
        fread(outData, 1, BLOCKMAP_FLAG_LENGTH, fp);
        if (memcmp(outData, BLOCKMAPPEND, strlen(BLOCKMAPPEND)) == 0) {
            if (memset_s(outData, len, 0, len) != EOK) {
                NUMBER_IDENTITY_LOGE("ReadBlockConfigToBuffer failed to memset_s");
                return NUMBER_IDENTITY_ERR_MEMSET_FAIL;
            }
            fseek(fp, -(BLOCKMAP_DATA_LENGTH + BLOCKMAP_FLAG_LENGTH), SEEK_END);
            fread(outData, 1, BLOCKMAP_DATA_LENGTH, fp);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        } else {
            NUMBER_IDENTITY_LOGE("memcmp BLOCKMAPPEND failed!");
            return NUMBER_IDENTITY_ERROR;
        }
    } else {
        NUMBER_IDENTITY_LOGE("memcmp BLOCKMAPPSTART failed!");
        return NUMBER_IDENTITY_ERROR;
    }
}

int32_t NumberLocationDbParse::ReadMobileOpFlagPos(FILE *fp)
{
    BlockMapStr blockMappStrTemp;
    fseek(fp, -(BLOCKMAP_DATA_LENGTH + BLOCKMAP_FLAG_LENGTH), SEEK_END);
    fseek(fp, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_HAS_MOBILE_OP) * sizeof(BlockMapStr), SEEK_CUR);
    fread(&blockMappStrTemp, sizeof(BlockMapStr), 1, fp);
    return blockMappStrTemp.blockFilePos;
}

int32_t NumberLocationDbParse::ReadMobileOpIndexPos(FILE *fp)
{
    BlockMapStr blockMappStrTemp;
    fseek(fp, -(BLOCKMAP_DATA_LENGTH + BLOCKMAP_FLAG_LENGTH), SEEK_END);
    fseek(fp, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_MOBILE_OP_INDEX) * sizeof(BlockMapStr), SEEK_CUR);
    fread(&blockMappStrTemp, sizeof(BlockMapStr), 1, fp);
    return blockMappStrTemp.blockFilePos;
}

int32_t NumberLocationDbParse::ReadMobileOpDataPos(FILE *fp)
{
    BlockMapStr blockMappStrTemp;
    fseek(fp, -(BLOCKMAP_DATA_LENGTH + BLOCKMAP_FLAG_LENGTH), SEEK_END);
    fseek(fp, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_MOBILE_OP_DATA) * sizeof(BlockMapStr), SEEK_CUR);
    fread(&blockMappStrTemp, sizeof(BlockMapStr), 1, fp);
    return blockMappStrTemp.blockFilePos;
}

int32_t NumberLocationDbParse::ReadMobileOpNamesPos(FILE *fp)
{
    BlockMapStr blockMappStrTemp;
    fseek(fp, -(BLOCKMAP_DATA_LENGTH + BLOCKMAP_FLAG_LENGTH), SEEK_END);
    fseek(fp, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_MOBILE_OP_NAMES) * sizeof(BlockMapStr), SEEK_CUR);
    fread(&blockMappStrTemp, sizeof(BlockMapStr), 1, fp);
    return blockMappStrTemp.blockFilePos;
}

void NumberLocationDbParse::GetMobileOperator(FILE *fp, const char *phoneNumber, char *mobileOpName)
{
    if (memset_s(mobileOpName, MAX_MOBILE_OP_NAME_LEN, 0, MAX_MOBILE_OP_NAME_LEN) != EOK) {
        NUMBER_IDENTITY_LOGE("get_MobileOp failed to memset_s");
        return;
    }
    if (phoneNumber == nullptr || *phoneNumber == '\0') {
        NUMBER_IDENTITY_LOGE("phoneNumber is null!");
        return;
    }
    const char *pDeNumber = phoneNumber;
    if (*pDeNumber != '1') {
        return;
    }
    if (strlen(pDeNumber) < PREFIX_NUMBER_LENGTH) {
        NUMBER_IDENTITY_LOGE("lenth not enough!");
        return;
    }
    int32_t pos = ReadMobileOpFlagPos(fp);
    char hasMobileop = 0;
    fseek(fp, pos, SEEK_SET);
    fread(&hasMobileop, 1, sizeof(char), fp);
    if (hasMobileop == 0) {
        NUMBER_IDENTITY_LOGE("hasMobileop is zero!");
        return;
    }
    char tempOpName[MAX_MOBILE_OP_NAME_LEN];
    if (memset_s(tempOpName, MAX_MOBILE_OP_NAME_LEN, 0, MAX_MOBILE_OP_NAME_LEN) != EOK) {
        NUMBER_IDENTITY_LOGE("get_MobileOp failed to memset_s");
        return;
    }
    if (memcpy_s(tempOpName, sizeof(tempOpName), pDeNumber + 1, PREFIX_TWO_LENGTH) != EOK) {
        NUMBER_IDENTITY_LOGE("get_MobileOp failed to memset_s");
        return;
    }
    short mobileOpNameIndex = 0;
    if (ReadOpIndexAndOpNamePos(fp, tempOpName, pDeNumber, pos, mobileOpNameIndex) != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("read op index and op name pos err!");
        return;
    }
    fseek(fp, pos + mobileOpNameIndex * MAX_MOBILE_OP_NAME_LEN, SEEK_SET);
    fread(mobileOpName, MAX_MOBILE_OP_NAME_LEN, 1, fp);
    return;
}

int32_t NumberLocationDbParse::ReadOpIndexAndOpNamePos(
    FILE *fp, char *tempOpName, const char *pDeNumber, int32_t &pos, short &mobileOpNameIndex)
{
    int mopIndex = atoi(tempOpName);
    pos = ReadMobileOpIndexPos(fp);
    fseek(fp, pos + mopIndex * sizeof(short), SEEK_SET);
    fread(&mobileOpNameIndex, 1, sizeof(short), fp);
    if (mobileOpNameIndex >= static_cast<int32_t>(E_MOBILE_OPS::E_MOBILE_OP_MAX)) {
        if (strlen(pDeNumber) < NUMBER_LIMIT_OPERATOR_NAME_PART) {
            NUMBER_IDENTITY_LOGE("pDeNumber lenth is illegal!");
            return NUMBER_IDENTITY_ERROR;
        }
        if (memset_s(tempOpName, MAX_MOBILE_OP_NAME_LEN, 0, MAX_MOBILE_OP_NAME_LEN) != EOK) {
            NUMBER_IDENTITY_LOGE("tempOpName failed to memset_s");
            return NUMBER_IDENTITY_ERROR;
        }
        if (memcpy_s(tempOpName, MAX_MOBILE_OP_NAME_LEN, pDeNumber + PREFIX_NUMBER_LENGTH, OPERATOR_NUMBER_LENGTH) !=
            EOK) {
            NUMBER_IDENTITY_LOGE("tempOpName failed to memcpy_s");
            return NUMBER_IDENTITY_ERROR;
        }
        mopIndex = atoi(tempOpName);
        pos = ReadMobileOpDataPos(fp);
        fseek(fp, pos + (mobileOpNameIndex - static_cast<int32_t>(
            E_MOBILE_OPS::E_MOBILE_OP_MAX)) * static_cast<int32_t>(E_MOBILE_OPS::E_MOBILE_OP_MAX) *
            sizeof(short) + (mopIndex * sizeof(short)), SEEK_SET);
        fread(&mobileOpNameIndex, 1, sizeof(short), fp);
    }
    pos = ReadMobileOpNamesPos(fp);
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t NumberLocationDbParse::ReadMaxQuHaoNum(FILE *fp, void *configBuffer, int32_t bufferLenth)
{
    BlockMapStr blockMappStrTemp;
    NUMBER_IDENTITY_LOGD("ReadMaxQuHaoNum: configBuffer len = %{public}d", bufferLenth);
    ReadBlockById(configBuffer, static_cast<int32_t>(
        MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_MAXQUHAONUM), &blockMappStrTemp);
    int32_t temp;
    fseek(fp, blockMappStrTemp.blockFilePos, SEEK_SET);
    fread(&temp, 1, sizeof(temp), fp);
    return temp;
}

int32_t NumberLocationDbParse::GetCityIndexByQuHao(FILE *fileHndRf, void *configBuffer,
    unsigned char *QuHaoData, int32_t pos, int32_t readSize, unsigned char *outCityName)
{
    int findFlag = 0;
    char *ptTmp = nullptr;
    int i = 0;
    int32_t seekLength = pos;
    int32_t readDigitSize = sizeof(char) * NUMBER_LIMIT_MIN * readSize;
    char *ptQuHaoList = (char *)malloc(sizeof(char) * NUMBER_LIMIT_MIN * readSize);
    if (ptQuHaoList != nullptr) {
        fseek(fileHndRf, seekLength, SEEK_SET);
        fread(ptQuHaoList, 1, readDigitSize, fileHndRf);
        ptTmp = ptQuHaoList;
        GetCityIndexInner(i, readSize, ptTmp, QuHaoData, findFlag, ptQuHaoList);
        free(ptQuHaoList);
        if (findFlag == 1) {
            GetCityName(fileHndRf, configBuffer, i, (unsigned char *)outCityName);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        } else {
            return NUMBER_IDENTITY_ERROR;
        }
    }
    return NUMBER_IDENTITY_ERROR;
}

void NumberLocationDbParse::GetCityIndexInner(
    int &i, int32_t &readSize, char *ptTmp, unsigned char *QuHaoData, int &findFlag, char *ptQuHaoList)
{
    for (i = 0; i < readSize; i++) {
        if (ptTmp != nullptr) {
            if (memcmp(ptTmp, QuHaoData, sizeof(char) * NUMBER_LIMIT_MIN) == 0) {
                findFlag = 1;
                break;
            } else {
                ptTmp += NUMBER_LIMIT_MIN;
            }
        }
        if (i == readSize - 1) {
            if (QuHaoData[PREFIX_NUMBER_LENGTH] == '\0') {
                break;
            } else {
                QuHaoData[PREFIX_NUMBER_LENGTH] = '\0';
                ptTmp = ptQuHaoList;
                i = -1;
            }
        }
    }
}

int32_t NumberLocationDbParse::GetPrefixIndex(
    FILE *fileHndRf, void *configBuffer, const char *numberPtr, int32_t &prefixIndex)
{
    if (fileHndRf == nullptr) {
        NUMBER_IDENTITY_LOGE("fileHndRf is nullptr!");
        return NUMBER_IDENTITY_ERROR;
    }
    char prefixList[PREFIX_NUMBER_LIST];
    if (memset_s(prefixList, sizeof(prefixList), 0, sizeof(prefixList)) != EOK) {
        NUMBER_IDENTITY_LOGE("GetPrefixIndex fail to memset_s.");
        return NUMBER_IDENTITY_ERROR;
    }
    ReadPhoneNumberPrefixList(fileHndRf, configBuffer, CONFIG_BUFFER_LENGTH, prefixList);
    char tmp2Chars[PREFIX_NUMBER_LENGTH];
    if (memset_s(tmp2Chars, sizeof(tmp2Chars), 0, sizeof(tmp2Chars)) != EOK) {
        NUMBER_IDENTITY_LOGE("GetPrefixIndex fail to memset_s.");
        return NUMBER_IDENTITY_ERROR;
    }
    if (memcpy_s(tmp2Chars, PREFIX_TWO_LENGTH, numberPtr, PREFIX_TWO_LENGTH) != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s address fail");
        return NUMBER_IDENTITY_ERROR;
    }
    if (strlen(numberPtr) < NUMBER_LIMIT_MIN) {
        NUMBER_IDENTITY_LOGE("numberPtr is less than NUMBER_LIMIT_MIN");
        return NUMBER_IDENTITY_ERROR;
    }
    char *pt_forPrefix = nullptr;
    pt_forPrefix = strstr((char *)prefixList, (char *)tmp2Chars);
    if (pt_forPrefix != nullptr) {
        prefixIndex = pt_forPrefix - prefixList + 1;
        prefixIndex = (int32_t)(prefixIndex / PREFIX_TWO_LENGTH);
    } else {
        return NUMBER_IDENTITY_ERROR;
    }
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

std::string NumberLocationDbParse::QueryPhoneNumberLocation(const char *number)
{
    unsigned char configBuffer[CONFIG_BUFFER_LENGTH];
    FILE *fp = OpenDataFile();
    if (fp == nullptr) {
        return "";
    }
    if (ReadBlockConfigToBuffer(fp, configBuffer, CONFIG_BUFFER_LENGTH) != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("QueryPhoneNumberLocation failed to read config buffer.");
        fclose(fp);
        return "";
    }
    char numberPtr[MAX_MOBILE_PHONE_NUMBER_LENGTH];
    int res = memcpy_s(numberPtr, MAX_MOBILE_PHONE_NUMBER_LENGTH, number, strlen(number));
    if (res != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s address failed! res = %{public}d", res);
        fclose(fp);
        return "";
    }
    int32_t prefixIndex = -1;
    if (GetPrefixIndex(fp, configBuffer, numberPtr, prefixIndex) != NUMBER_IDENTITY_ERR_SUCCESS) {
        fclose(fp);
        return "";
    }
    char strNum[NUMBER_LIMIT_MIN + 1];
    if (memcpy_s(strNum, NUMBER_LIMIT_MIN, numberPtr + PREFIX_TWO_LENGTH, NUMBER_LIMIT_MIN) != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s address failed!");
        fclose(fp);
        return "";
    }
    short cityIndex;
    int32_t mobileNum = (int32_t)std::atoi(strNum);
    GetCityIndexByPhoneNum(fp, configBuffer, mobileNum, prefixIndex, &cityIndex);
    char cityNameUnicode[MAXCITYNAME_UNICODE_BUF];
    char unicodeChars1[MAXCITYNAME_UTF8_BUF];
    GetCityName(fp, configBuffer, cityIndex, (unsigned char *)cityNameUnicode);
    fclose(fp);
    TransformUnicodeToUTF8((char *)cityNameUnicode, (char *)unicodeChars1, MAXCITYNAME_UTF8_BUF);
    return unicodeChars1;
}

std::string NumberLocationDbParse::QueryOpNamebyPhoneNumber(const char *number)
{
    unsigned char configBuffer[CONFIG_BUFFER_LENGTH];
    FILE *fp = OpenDataFile();
    if (fp == nullptr) {
        NUMBER_IDENTITY_LOGE("open file err!");
        return "";
    }
    if (ReadBlockConfigToBuffer(fp, configBuffer, CONFIG_BUFFER_LENGTH) != NUMBER_IDENTITY_ERR_SUCCESS) {
        fclose(fp);
        NUMBER_IDENTITY_LOGE("read block config err!");
        return "";
    }
    char UTF8Chars[MAX_MOBILE_OP_NAME_LEN];
    char mobileOpName[MAX_MOBILE_OP_NAME_LEN];
    GetMobileOperator(fp, number, mobileOpName);
    TransformUnicodeToUTF8((char *)mobileOpName, (char *)UTF8Chars, MAX_MOBILE_OP_NAME_LEN);
    fclose(fp);
    return UTF8Chars;
}

std::string NumberLocationDbParse::QueryTelNumberLocation(const char *number)
{
    unsigned char configBuffer[CONFIG_BUFFER_LENGTH];
    FILE *fp = OpenDataFile();
    if (fp == nullptr) {
        NUMBER_IDENTITY_LOGE("open file err!");
        return "";
    }
    if (ReadBlockConfigToBuffer(fp, configBuffer, CONFIG_BUFFER_LENGTH) != NUMBER_IDENTITY_ERR_SUCCESS) {
        fclose(fp);
        NUMBER_IDENTITY_LOGE("read block config err!");
        return "";
    }
    char strNum[NUMBER_LIMIT_CITY_PART + 1];
    char cityNameUnicode[MAXCITYNAME_UNICODE_BUF];
    int32_t n = std::strlen((const char *)number);
    if (n < PREFIX_NUMBER_LENGTH) {
        fclose(fp);
        NUMBER_IDENTITY_LOGE("length is not long enough!");
        return "";
    }
    if (memcpy_s(strNum, NUMBER_LIMIT_CITY_PART, number, NUMBER_LIMIT_CITY_PART) != EOK) {
        fclose(fp);
        return "";
    }
    int32_t maxQuHaoNum = ReadMaxQuHaoNum(fp, configBuffer, CONFIG_BUFFER_LENGTH);
    int32_t pos = ReadBlockFilePosition(
        configBuffer, CONFIG_BUFFER_LENGTH, static_cast<int32_t>(MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_QUHAODATA));
    int32_t rs = GetCityIndexByQuHao(fp, configBuffer, (unsigned char *)strNum, pos, maxQuHaoNum,
        (unsigned char *)cityNameUnicode);
    if (rs != NUMBER_IDENTITY_ERR_SUCCESS) {
        fclose(fp);
        return "";
    }
    fclose(fp);
    char unicodeChars[MAXCITYNAME_UTF8_BUF];
    TransformUnicodeToUTF8(cityNameUnicode, unicodeChars, MAXCITYNAME_UTF8_BUF);
    return unicodeChars;
}

std::string NumberLocationDbParse::QueryAreaCodebyPhoneNumber(const char *number)
{
    unsigned char configBuffer[CONFIG_BUFFER_LENGTH];
    FILE *fp = OpenDataFile();
    if (fp == nullptr) {
        NUMBER_IDENTITY_LOGE("open file err!");
        return "";
    }
    if (ReadBlockConfigToBuffer(fp, configBuffer, CONFIG_BUFFER_LENGTH) != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("QueryAreaCodebyPhoneNumber failed to read config buffer.");
        fclose(fp);
        return "";
    }
    char numberPtr[maxNumberLen];
    if (memcpy_s(numberPtr, maxNumberLen, number, strlen(number)) != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s address failed!");
        fclose(fp);
        return "";
    }
    int32_t prefixIndex = -1;
    if (GetPrefixIndex(fp, configBuffer, numberPtr, prefixIndex) != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("GetPrefixIndex failed!");
        fclose(fp);
        return "";
    }
    char strNum[NUMBER_LIMIT_MIN + 1];
    if (memcpy_s(strNum, NUMBER_LIMIT_MIN, numberPtr + PREFIX_TWO_LENGTH, NUMBER_LIMIT_MIN) != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s address failed!");
        fclose(fp);
        return "";
    }
    short cityIndex;
    int32_t mobileNum = (int32_t)std::atoi(strNum);
    GetCityIndexByPhoneNum(fp, configBuffer, mobileNum, prefixIndex, &cityIndex);
    char quHao[QUHAO_LENGTH_LIMIT];
    ReadMaxQuHaoNum(fp, configBuffer, CONFIG_BUFFER_LENGTH);
    int32_t pos = ReadBlockFilePosition(
        configBuffer, CONFIG_BUFFER_LENGTH, static_cast<int32_t>(MMIAPI_DEV_GET_CTRL_ACTION::BLOCK_ID_QUHAODATA));
    int32_t seek_length;
    int32_t read_size;
    seek_length = pos + sizeof(unsigned char) * NUMBER_LIMIT_MIN * cityIndex;
    read_size = sizeof(unsigned char) * NUMBER_LIMIT_MIN;
    fseek(fp, seek_length, SEEK_SET);
    fread(quHao, 1, read_size, fp);
    fclose(fp);
    return quHao;
}

bool NumberLocationDbParse::LoadNumberLocationData()
{
    if (dataLoaded_) {
        NUMBER_IDENTITY_LOGD("LoadNumberLocationData: data already loaded, total_records=%zu", 
                             locationDataSet_.records.size());
        return true;
    }
    
    NUMBER_IDENTITY_LOGI("LoadNumberLocationData: starting to load data file");
    const char *path = NUMBER_LOCATION_DATA_PATH_UPDATE;
    NUMBER_IDENTITY_LOGD("LoadNumberLocationData: trying path: %{public}s", path);
    std::ifstream file(path);
    if (file.fail()) {
        NUMBER_IDENTITY_LOGW("LoadNumberLocationData: fopen NUMBER_LOCATION_DATA_PATH_UPDATE fail, try default path.");
        path = NUMBER_LOCATION_DATA_PATH;
        NUMBER_IDENTITY_LOGD("LoadNumberLocationData: trying default path: %{public}s", path);
        file.open(path);
    }
    
    if (file.fail()) {
        NUMBER_IDENTITY_LOGE("LoadNumberLocationData: open numberlocation.data file fail, errno: %{public}d: %{public}s", 
                              errno, strerror(errno));
        return false;
    }
    
    NUMBER_IDENTITY_LOGI("LoadNumberLocationData: file opened successfully, starting parse");
    NumberLocationParser parser;
    if (!parser.Parse(file, locationDataSet_)) {
        NUMBER_IDENTITY_LOGE("LoadNumberLocationData: Parse numberlocation.data failed.");
        file.close();
        return false;
    }
    
    file.close();
    dataLoaded_ = true;
    NUMBER_IDENTITY_LOGI("LoadNumberLocationData: success, version: %{public}s, total_records: %zu", 
                          locationDataSet_.version.c_str(), locationDataSet_.records.size());
    
    return true;
}

std::string NumberLocationDbParse::QueryLocationFromDataFile(const char *number)
{
    NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile called with number: %{public}s", number ? number : "null");
    
    if (number == nullptr || strlen(number) < PREFIX_NUMBER_LENGTH) {
        NUMBER_IDENTITY_LOGW("QueryLocationFromDataFile: invalid number (null or length < 3)");
        return "";
    }
    
    if (!LoadNumberLocationData()) {
        NUMBER_IDENTITY_LOGE("QueryLocationFromDataFile: LoadNumberLocationData failed");
        return "";
    }
    
    std::string numberStr(number);
    NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile: querying number=%{public}s, length=%zu, total_records=%zu", 
                         numberStr.c_str(), numberStr.length(), locationDataSet_.records.size());
    
    const NumberLocationRecord *record = nullptr;
    std::string matchedPrefix = "";
    
    // Try to match 7-digit prefix first
    if (numberStr.length() >= 7) {
        std::string prefix7 = numberStr.substr(0, 7);
        NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile: trying 7-digit prefix: %{public}s", prefix7.c_str());
        auto it = locationDataSet_.records.find(prefix7);
        if (it != locationDataSet_.records.end()) {
            record = &(it->second);
            matchedPrefix = prefix7;
            NUMBER_IDENTITY_LOGI("QueryLocationFromDataFile: matched 7-digit prefix %{public}s -> %{public}s %{public}s %{public}s", 
                                  prefix7.c_str(), record->province.c_str(), record->city.c_str(), record->operator_.c_str());
        } else {
            NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile: 7-digit prefix %{public}s not found", prefix7.c_str());
        }
    }
    
    // Then try to match 3-digit prefix
    if (record == nullptr && numberStr.length() >= 3) {
        std::string prefix3 = numberStr.substr(0, 3);
        NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile: trying 3-digit prefix: %{public}s", prefix3.c_str());
        auto it = locationDataSet_.records.find(prefix3);
        if (it != locationDataSet_.records.end()) {
            record = &(it->second);
            matchedPrefix = prefix3;
            NUMBER_IDENTITY_LOGI("QueryLocationFromDataFile: matched 3-digit prefix %{public}s -> %{public}s %{public}s %{public}s", 
                                  prefix3.c_str(), record->province.c_str(), record->city.c_str(), record->operator_.c_str());
        } else {
            NUMBER_IDENTITY_LOGD("QueryLocationFromDataFile: 3-digit prefix %{public}s not found", prefix3.c_str());
        }
    }
    
    if (record == nullptr) {
        NUMBER_IDENTITY_LOGW("QueryLocationFromDataFile: no record found for number %{public}s", numberStr.c_str());
        return "";
    }
    
    // Format return: Province City (without space)
    std::string result;
    if (!record->province.empty() && !record->city.empty()) {
        if (record->province == record->city) {
            result = record->province;
        } else {
            result = record->province + record->city;
        }
        NUMBER_IDENTITY_LOGI("QueryLocationFromDataFile: result (province+city) = %{public}s", result.c_str());
    } else if (!record->location.empty()) {
        result = record->location;
        NUMBER_IDENTITY_LOGI("QueryLocationFromDataFile: result (location) = %{public}s", result.c_str());
    } else {
        NUMBER_IDENTITY_LOGW("QueryLocationFromDataFile: record found but province/city/location all empty for prefix %{public}s", 
                             matchedPrefix.c_str());
    }
    
    return result;
}

std::string NumberLocationDbParse::QueryOperatorFromDataFile(const char *number)
{
    NUMBER_IDENTITY_LOGD("QueryOperatorFromDataFile called with number: %{public}s", number ? number : "null");
    
    if (number == nullptr || strlen(number) < PREFIX_NUMBER_LENGTH) {
        NUMBER_IDENTITY_LOGW("QueryOperatorFromDataFile: invalid number (null or length < 3)");
        return "";
    }
    
    if (!LoadNumberLocationData()) {
        NUMBER_IDENTITY_LOGE("QueryOperatorFromDataFile: LoadNumberLocationData failed");
        return "";
    }
    
    std::string numberStr(number);
    
    // Try to match 7-digit prefix first
    if (numberStr.length() >= 7) {
        std::string prefix7 = numberStr.substr(0, 7);
        NUMBER_IDENTITY_LOGD("QueryOperatorFromDataFile: trying 7-digit prefix: %{public}s", prefix7.c_str());
        auto it = locationDataSet_.records.find(prefix7);
        if (it != locationDataSet_.records.end()) {
            NUMBER_IDENTITY_LOGI("QueryOperatorFromDataFile: matched 7-digit prefix %{public}s -> operator: %{public}s", 
                                  prefix7.c_str(), it->second.operator_.c_str());
            return it->second.operator_;
        }
    }
    
    // Then try to match 3-digit prefix
    if (numberStr.length() >= 3) {
        std::string prefix3 = numberStr.substr(0, 3);
        NUMBER_IDENTITY_LOGD("QueryOperatorFromDataFile: trying 3-digit prefix: %{public}s", prefix3.c_str());
        auto it = locationDataSet_.records.find(prefix3);
        if (it != locationDataSet_.records.end()) {
            NUMBER_IDENTITY_LOGI("QueryOperatorFromDataFile: matched 3-digit prefix %{public}s -> operator: %{public}s", 
                                  prefix3.c_str(), it->second.operator_.c_str());
            return it->second.operator_;
        }
    }
    
    NUMBER_IDENTITY_LOGW("QueryOperatorFromDataFile: no operator found for number %{public}s", numberStr.c_str());
    return "";
}

// New method: Query complete location information (Province City Operator)
std::string NumberLocationDbParse::QueryFullLocationFromDataFile(const char *number)
{
    NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile called with number: %{public}s", number ? number : "null");
    
    if (number == nullptr || strlen(number) < PREFIX_NUMBER_LENGTH) {
        NUMBER_IDENTITY_LOGW("QueryFullLocationFromDataFile: invalid number (null or length < 3)");
        return "";
    }
    
    if (!LoadNumberLocationData()) {
        NUMBER_IDENTITY_LOGE("QueryFullLocationFromDataFile: LoadNumberLocationData failed");
        return "";
    }
    
    std::string numberStr(number);
    NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile: querying number=%{public}s, length=%zu", 
                         numberStr.c_str(), numberStr.length());
    
    const NumberLocationRecord *record = nullptr;
    std::string matchedPrefix = "";
    
    // Try to match 7-digit prefix first
    if (numberStr.length() >= 7) {
        std::string prefix7 = numberStr.substr(0, 7);
        NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile: trying 7-digit prefix: %{public}s", prefix7.c_str());
        auto it = locationDataSet_.records.find(prefix7);
        if (it != locationDataSet_.records.end()) {
            record = &(it->second);
            matchedPrefix = prefix7;
            NUMBER_IDENTITY_LOGI("QueryFullLocationFromDataFile: matched 7-digit prefix %{public}s -> %{public}s %{public}s %{public}s", 
                                  prefix7.c_str(), record->province.c_str(), record->city.c_str(), record->operator_.c_str());
        }
    }
    
    // Then try to match 3-digit prefix
    if (record == nullptr && numberStr.length() >= 3) {
        std::string prefix3 = numberStr.substr(0, 3);
        NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile: trying 3-digit prefix: %{public}s", prefix3.c_str());
        auto it = locationDataSet_.records.find(prefix3);
        if (it != locationDataSet_.records.end()) {
            record = &(it->second);
            matchedPrefix = prefix3;
            NUMBER_IDENTITY_LOGI("QueryFullLocationFromDataFile: matched 3-digit prefix %{public}s -> %{public}s %{public}s %{public}s", 
                                  prefix3.c_str(), record->province.c_str(), record->city.c_str(), record->operator_.c_str());
        }
    }
    
    if (record == nullptr) {
        NUMBER_IDENTITY_LOGW("QueryFullLocationFromDataFile: no record found for number %{public}s", numberStr.c_str());
        return "";
    }
    
    // Format return: Province City Operator (without space between province and city)
    std::string result;
    
    // Build province and city information (without space)
    std::string locationPart;
    if (!record->province.empty() && !record->city.empty()) {
        if (record->province == record->city) {
            locationPart = record->province;
        } else {
            locationPart = record->province + record->city;
        }
        NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile: locationPart (province+city) = %{public}s", locationPart.c_str());
    } else if (!record->location.empty()) {
        locationPart = record->location;
        NUMBER_IDENTITY_LOGD("QueryFullLocationFromDataFile: locationPart (location) = %{public}s", locationPart.c_str());
    }
    
    // Add operator information
    // Priority: location + operator > operator only > location only
    if (!locationPart.empty() && !record->operator_.empty()) {
        result = locationPart + " " + record->operator_;
        NUMBER_IDENTITY_LOGI("QueryFullLocationFromDataFile: result (location + operator) = %{public}s", result.c_str());
    } else if (!record->operator_.empty()) {
        // If location not found but operator found, show operator only
        result = record->operator_;
        NUMBER_IDENTITY_LOGI("QueryFullLocationFromDataFile: result (operator only) = %{public}s", result.c_str());
    } else if (!locationPart.empty()) {
        result = locationPart;
        NUMBER_IDENTITY_LOGI("QueryFullLocationFromDataFile: result (location only) = %{public}s", result.c_str());
    } else {
        NUMBER_IDENTITY_LOGW("QueryFullLocationFromDataFile: record found but all fields empty for prefix %{public}s", 
                             matchedPrefix.c_str());
    }
    
    return result;
}
} // namespace Telephony
} // namespace OHOS
