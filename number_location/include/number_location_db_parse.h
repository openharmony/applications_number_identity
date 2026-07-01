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

#ifndef NUMBER_LCATION_DB_PARSE_H
#define NUMBER_LCATION_DB_PARSE_H

#include "number_identity_inner_type.h"
#include "pac_map.h"
#include "singleton.h"
#include "number_location_parser.h"

namespace OHOS {
namespace Telephony {
class NumberLocationDbParse {
    DECLARE_DELAYED_SINGLETON(NumberLocationDbParse)
public:
    std::string QueryPhoneNumberLocation(const char *number);
    std::string QueryTelNumberLocation(const char *number);
    std::string QueryOpNamebyPhoneNumber(const char *number);
    std::string QueryAreaCodebyPhoneNumber(const char *number);
    std::string QueryLocationFromDataFile(const char *number);
    std::string QueryOperatorFromDataFile(const char *number);
    std::string QueryFullLocationFromDataFile(const char *number);

private:
    bool LoadNumberLocationData();
    NumberLocationDataSet locationDataSet_;
    bool dataLoaded_;
    FILE *OpenDataFile();
    int32_t ReadBlockConfigToBuffer(FILE *fp, void *outData, int32_t len);
    void ReadBlockById(void *data, int32_t blockId, BlockMapStr *outData);
    void ReadPhoneNumberPrefixList(FILE *fp, void *configBuffer, int32_t bufferLenth, char *outData);
    void GetCityIndexByPhoneNum(
        FILE *fileHndRf, void *configBuffer, int32_t phonNumIndex, int32_t prefixIndex, void *rtData);
    int32_t ReadBlockFilePosition(void *configBuffer, int32_t bufferLenth, int32_t blockId);
    int32_t AsciiToNum(char *pChar);
    void GetCityName(FILE *fileHndRf, void *configBuffer, int32_t cityIndex, unsigned char *outCityName);
    void TransformUnicodeToUTF8(char *cityNameUnicode, char *cityNameUTF8, int32_t lenthUTF8);
    char GetEndNum(wchar_t *pText, int32_t len);
    char GetActualNum(wchar_t *pText, int32_t len);
    void UnicodeToUTF_8(char *pOut, wchar_t *pText);
    void GetMobileOperator(FILE *fp, const char *phoneno, char *mobileOpName);
    int32_t ReadMobileOpFlagPos(FILE *fp);
    int32_t ReadMobileOpIndexPos(FILE *fp);
    int32_t ReadMobileOpDataPos(FILE *fp);
    int32_t ReadMobileOpNamesPos(FILE *fp);
    int32_t ReadMaxQuHaoNum(FILE *fp, void *configBuffer, int32_t bufferLenth);
    int32_t GetCityIndexByQuHao(FILE *fileHndRf, void *configBuffer, unsigned char *QuHaoData, int32_t pos,
        int32_t readSize, unsigned char *outCityName);
    void GetCityIndexInner(
        int &i, int32_t &readSize, char *ptTmp, unsigned char *QuHaoData, int &findFlag, char *ptQuHaoList);
    int32_t GetPrefixIndex(FILE *fileHndRf, void *configBuffer, const char *number, int32_t &prefixIndex);
    int32_t ReadOpIndexAndOpNamePos(
        FILE *fp, char *tempOpName, const char *pDeNumber, int32_t &pos, short &mobileOpNameIndex);
};
} // namespace Telephony
} // namespace OHOS

#endif // NUMBER_LCATION_DB_PARSE_H