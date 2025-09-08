﻿#include "Input.h"
#include "StringUtility.h"

#include <DxLib.h>
#include <cassert>

namespace {
    const std::string kKeyConfigSignature = "kcfg";
    const std::string kKeyConfigFilename = "keyconfig.dat";
}

Input& Input::GetInstance() {
    // この宣言の時点でメモリが確保されてアプリ終了まで残る
    static Input input;
    return input;
}

void Input::Update() {
    _last = _current;

    // 直前の情報をコピー
    std::copy(std::begin(_currentRawKeybdState), std::end(_currentRawKeybdState), std::begin(_lastRawKeybdState));
    _lastRawPadState = _currentRawPadState;
    // ハードウェア(周辺機器)の状態を取得
    GetHitKeyStateAll(_currentRawKeybdState);//ハードウェアから現在の入力状態を取得
    _currentRawPadState = GetJoypadInputState(DX_INPUT_PAD1);//パッド１の状態を取得

    //入力チェック(生の入力をゲームのイベントに変換していく)
    for (const auto& inputRow : _inputTable) {
        // 行のキー
        const std::string& key = inputRow.first;
        // 「押してる状態」の初期化
        if (_current.find(key) != _current.end()) {
            _current[key] = false;
        }
        
        PeripheralType inputType = _lastInputType;

        // 入力定義vectorのループ
        for (const auto& hardInput : inputRow.second) {
            if (hardInput.type == PeripheralType::keybd) {
                _current[key] = _currentRawKeybdState[hardInput.id];
            }
            else if (hardInput.type == PeripheralType::pad1) {
                _current[key] = hardInput.id & _currentRawPadState;
            }
            // どれか一つでも「押されている」状態ならもう調べない
            // 機器の情報が欲しいわけではない為、誰かが押されていればもうOK
            if (_current[key]) {
                inputType = hardInput.type;
                break;
            }
        }

        _lastInputType = inputType;
    }

    // 左右スティック更新
    int xInput, zInput;
    GetJoypadAnalogInputRight(&xInput, &zInput, DX_INPUT_PAD1);
    _currentRightStickInput = { static_cast<float>(xInput), 0, static_cast<float>(zInput) };
    if (_currentRightStickInput.SqrMagnitude() != 0.0f) {  // 入力があった場合更新
        _lastRightStickInput = _currentRightStickInput;
    }
    GetJoypadAnalogInput(&xInput, &zInput, DX_INPUT_PAD1);
    _currentLeftStickInput = { static_cast<float>(xInput), 0, static_cast<float>(zInput) };
    if (_currentLeftStickInput.SqrMagnitude() != 0.0f) {   // 入力があった場合更新
        _lastLeftStickInput = _currentLeftStickInput;
    }

    // マウスボタン更新
    _lastRawMouseState = _currentRawMouseState;
    _currentRawMouseState = GetMouseInput();
    // マウス位置更新
    _lastMousePosition = _currentMousePosition;
    GetMousePoint(&xInput, &zInput);
    _currentMousePosition = { static_cast<float>(xInput), 0, static_cast<float>(zInput) };
}

bool Input::IsPress(const char* key) const {
    // そもそも登録されていないキータイプならreturn
    if (!_current.contains(key)) {
        assert(false && "不明な名前");
        return false;
    }
    // 押されているかどうかを返す
    return _current.at(key);

    // (旧処理)
    //// 押されていればtrue
    //return (_padInput & button);
}

bool Input::IsTrigger(const char* key) const {
    // そもそも登録されていないキータイプならreturn
    if (!_current.contains(key)) {
        assert(false && "不明な名前");
        return false;
    }
    // 1f前は押されてない かつ 今押されている ならtrue
    return (_current.at(key) && !_last.at(key));

    // (旧処理)
    //// 押されていればtrue.そうでないならfalse
    //bool isNow = (_padInput & button);
    //bool isLast = (_lastInput & button);
    //return (isNow && !isLast);
}

Vector3 Input::GetPadRightSitck() const {
    return _currentRightStickInput;
}

Vector3 Input::GetPadRightSitckLast() const {
    return _lastRightStickInput;
}


Vector3 Input::GetPadLeftSitck() const {
    return _currentLeftStickInput;
}

Vector3 Input::GetPadLeftSitckLast() const {
    return _lastLeftStickInput;
}

bool Input::IsPressMouseRightClick() const
{
    return _currentRawMouseState & MOUSE_INPUT_RIGHT;
}

bool Input::IsPressMouseLeftClick() const
{
    return _currentRawMouseState & MOUSE_INPUT_LEFT;
}

bool Input::IsPressMouseMiddleClick() const
{
    return _currentRawMouseState & MOUSE_INPUT_MIDDLE;
}

bool Input::IsTriggerMouseRightClick() const
{
    return (_currentRawMouseState & MOUSE_INPUT_RIGHT && 
        !(_lastRawMouseState & MOUSE_INPUT_RIGHT));
}

bool Input::IsTriggerMouseLeftClick() const
{
    return (_currentRawMouseState & MOUSE_INPUT_LEFT &&
        !(_lastRawMouseState & MOUSE_INPUT_LEFT));
}

bool Input::IsTriggerMouseMiddleClick() const
{
    return (_currentRawMouseState & MOUSE_INPUT_MIDDLE &&
        !(_lastRawMouseState & MOUSE_INPUT_MIDDLE));
}

Vector3 Input::GetMousePosition() const
{
    return _currentMousePosition;
}

Vector3 Input::GetMousePositionLast() const
{
    return _lastMousePosition;
}

void Input::SetInputType(PeripheralType type)
{
    _lastInputType = type;
}

Input::PeripheralType Input::GetLastInputType() const
{
    return _lastInputType;
}

Input::Input() :
    _inputTable(),
    _tempInputTable(),
    _current(),
    _last(),
    _orderForDisplay(),
    _lastInputType(Input::PeripheralType::pad1),
    _currentRawPadState(),
    _lastRawPadState(),
    _currentRawKeybdState(),
    _lastRawKeybdState(),
    _currentRightStickInput(),
    _lastRightStickInput(),
    _currentLeftStickInput(),
    _lastLeftStickInput(),
    _currentRawMouseState(),
    _lastRawMouseState(),
    _currentMousePosition(),
    _lastMousePosition()
{
    SetDefault();
    LoadInputTable();
    // 一時テーブルにコピー
    _tempInputTable = _inputTable;
    // 表示順序初期化
    _orderForDisplay = { "ok","next","back","pause","action","dash","jump" };
}

void Input::SetDefault()
{
    // タイトルシーンで使用するボタンテーブル
    _inputTable["Title:ChangeGameScene"] = { {PeripheralType::keybd, KEY_INPUT_RETURN},
                        {PeripheralType::pad1, PAD_INPUT_1}         // Aボタン
    };
    _inputTable["Title:ChangeInstructionScene"] = { {PeripheralType::keybd, KEY_INPUT_ESCAPE},
                        {PeripheralType::pad1, PAD_INPUT_2}         // Bボタン
    };

    // 操作説明シーンで使用するボタンテーブル
    _inputTable["Instruction:ChangeGameScene"] = { {PeripheralType::keybd, KEY_INPUT_RETURN},
                        {PeripheralType::pad1, PAD_INPUT_1}         // Aボタン
    };
    _inputTable["Instruction:ChangeTitleScene"] = { {PeripheralType::keybd, KEY_INPUT_ESCAPE},
                        {PeripheralType::pad1, PAD_INPUT_2}         // Bボタン
    };

    // ゲームシーンで使用するボタンテーブル
    _inputTable["Gameplay:Attack"] = { {PeripheralType::keybd, KEY_INPUT_Z},
                            {PeripheralType::pad1, PAD_INPUT_1}     // Aボタン
    };
    _inputTable["Gameplay:Dash"] = { {PeripheralType::keybd, KEY_INPUT_X},
                        {PeripheralType::pad1, PAD_INPUT_3}         // Xボタン
    };
    _inputTable["Gameplay:Jump"] = { {PeripheralType::keybd, KEY_INPUT_C},
                        {PeripheralType::pad1, PAD_INPUT_1}         // Aボタン
    };

    _inputTable["Gameplay:Up"] = { {PeripheralType::keybd, KEY_INPUT_W},
                            {PeripheralType::pad1, PAD_INPUT_UP}
    };
    _inputTable["Gameplay:Down"] = { {PeripheralType::keybd, KEY_INPUT_S},
                            {PeripheralType::pad1, PAD_INPUT_DOWN}
    };
    _inputTable["Gameplay:Left"] = { {PeripheralType::keybd, KEY_INPUT_A},
                            {PeripheralType::pad1, PAD_INPUT_LEFT}
    };
    _inputTable["Gameplay:Right"] = { {PeripheralType::keybd, KEY_INPUT_D},
                            {PeripheralType::pad1, PAD_INPUT_RIGHT}
    };
    _inputTable["Gameplay:Enter"] = { {PeripheralType::keybd, KEY_INPUT_RETURN},
                            {PeripheralType::pad1, PAD_INPUT_1}         // Aボタン
    };
    
    // リザルトシーンで使用するボタンテーブル
    _inputTable["Result:ChangeGameScene"] = { {PeripheralType::keybd, KEY_INPUT_RETURN},
                        {PeripheralType::pad1, PAD_INPUT_1}         // Aボタン
    };
    _inputTable["Result:ChangeTitleScene"] = { {PeripheralType::keybd, KEY_INPUT_ESCAPE},
                        {PeripheralType::pad1, PAD_INPUT_2}         // Bボタン
    };
    
    // デバッグ用
    _inputTable["Debug::Exit1"] = { {PeripheralType::keybd, KEY_INPUT_ESCAPE},
                            {PeripheralType::pad1, PAD_INPUT_8}     // RStartボタン
    };
    _inputTable["Debug::Exit2"] = { {PeripheralType::keybd, KEY_INPUT_RETURN},
                            {PeripheralType::pad1, PAD_INPUT_7}     // LStartボタン
    };
    _inputTable["Debug::NextScene1"] = { {PeripheralType::keybd, KEY_INPUT_P},
                            {PeripheralType::pad1, PAD_INPUT_8}     // RStartボタン
    };
    _inputTable["Debug::NextScene2"] = { {PeripheralType::keybd, KEY_INPUT_O},
                            {PeripheralType::pad1, PAD_INPUT_7}     // LStartボタン
    };

    
    _tempInputTable = _inputTable;  // 一時テーブルにコピー

    /*
    { PAD_INPUT_1,      L"PAD_INPUT_1 (Logi A)" },
    { PAD_INPUT_2,      L"PAD_INPUT_2 (Logi B)" },
    { PAD_INPUT_3,      L"PAD_INPUT_3 (Logi X)" },
    { PAD_INPUT_4,      L"PAD_INPUT_4 (Logi Y)" },
    { PAD_INPUT_5,      L"PAD_INPUT_5 (Logi LB)" },
    { PAD_INPUT_6,      L"PAD_INPUT_6 (Logi RB)" },
    { PAD_INPUT_7,      L"PAD_INPUT_7 (Logi LStart)" },
    { PAD_INPUT_8,      L"PAD_INPUT_8 (Logi RStart)" },
    { PAD_INPUT_9,      L"PAD_INPUT_9 (Logi LStick)" },
    { PAD_INPUT_10,     L"PAD_INPUT_10 (Logi RStick)" },
    { PAD_INPUT_11,     L"PAD_INPUT_11" },
    { PAD_INPUT_12,     L"PAD_INPUT_12" },

    { PAD_INPUT_13,     L"PAD_INPUT_13" },
    { PAD_INPUT_14,     L"PAD_INPUT_14" },
    { PAD_INPUT_15,     L"PAD_INPUT_15" },
    { PAD_INPUT_16,     L"PAD_INPUT_16" },
    { PAD_INPUT_17,     L"PAD_INPUT_17" },
    { PAD_INPUT_18,     L"PAD_INPUT_18" },
    { PAD_INPUT_19,     L"PAD_INPUT_19" },
    { PAD_INPUT_20,     L"PAD_INPUT_20" },
    { PAD_INPUT_21,     L"PAD_INPUT_21" },
    { PAD_INPUT_22,     L"PAD_INPUT_22" },
    { PAD_INPUT_23,     L"PAD_INPUT_23" },
    { PAD_INPUT_24,     L"PAD_INPUT_24" },
    { PAD_INPUT_25,     L"PAD_INPUT_25" },
    { PAD_INPUT_26,     L"PAD_INPUT_26" },
    { PAD_INPUT_27,     L"PAD_INPUT_27" },
    { PAD_INPUT_28,     L"PAD_INPUT_28" },

    // 方向キー
    { PAD_INPUT_UP,     L"PAD_INPUT_UP" },
    { PAD_INPUT_DOWN,   L"PAD_INPUT_DOWN" },
    { PAD_INPUT_LEFT,   L"PAD_INPUT_LEFT" },
    { PAD_INPUT_RIGHT,  L"PAD_INPUT_RIGHT" },

    // アルファベットボタン
    { PAD_INPUT_A,      L"PAD_INPUT_A (Logi A)" },
    { PAD_INPUT_B,      L"PAD_INPUT_B (Logi B)" },
    { PAD_INPUT_C,      L"PAD_INPUT_C (Logi X)" },
    { PAD_INPUT_D,      L"PAD_INPUT_D" },
    //{ PAD_INPUT_E,      L"PAD_INPUT_E" },
    { PAD_INPUT_F, L"PAD_INPUT_F" },
    { PAD_INPUT_G,      L"PAD_INPUT_G" },
    { PAD_INPUT_H,      L"PAD_INPUT_H" },
    { PAD_INPUT_I,      L"PAD_INPUT_I" },
    { PAD_INPUT_J,      L"PAD_INPUT_J" },
    { PAD_INPUT_K,      L"PAD_INPUT_K" },
    { PAD_INPUT_L,      L"PAD_INPUT_L (Logi LStart)" },
    { PAD_INPUT_M,      L"PAD_INPUT_M (Logi RStick)" },
    { PAD_INPUT_N,      L"PAD_INPUT_N" },
    { PAD_INPUT_O,      L"PAD_INPUT_O" },
    { PAD_INPUT_P,      L"PAD_INPUT_P" },
    //{ PAD_INPUT_Q,      L"PAD_INPUT_Q" },
    { PAD_INPUT_R,      L"PAD_INPUT_R (Logi RStart)" },
    { PAD_INPUT_S,      L"PAD_INPUT_S" },
    { PAD_INPUT_T,      L"PAD_INPUT_T" },
    { PAD_INPUT_U,      L"PAD_INPUT_U" },
    { PAD_INPUT_V,      L"PAD_INPUT_V" },
    { PAD_INPUT_W,      L"PAD_INPUT_W" },
    { PAD_INPUT_X,      L"PAD_INPUT_X (Logi Y)" },
    { PAD_INPUT_Y,      L"PAD_INPUT_Y (Logi LB)" },
    { PAD_INPUT_Z,      L"PAD_INPUT_Z (Logi RB)" },

        // その他
    { PAD_INPUT_START,  L"PAD_INPUT_START (Logi LStick)" },
    { PAD_INPUT_LL,     L"PAD_INPUT_LL" },
    { PAD_INPUT_RR,     L"PAD_INPUT_RR" },
    { PAD_INPUT_XX,     L"PAD_INPUT_XX" },
    */
}

int Input::GetKeyboradState() const
{
    // 全チェックし、どれかひとつでも入力があったらそれを返す
    for (int i = 0; i < _countof(_currentRawKeybdState); ++i) {
        if (_currentRawKeybdState[i] && !_lastRawKeybdState[i]) {
            return i;
        }
    }
    return -1;
}

int Input::GetPadState(int padno) const
{
    // 指定番号のパッド情報をを取得する
    uint32_t bit = 0b00000000000000000000000000000001;
    for (int i = 0; i < 32; ++i) {
        auto value = (bit << i);
        if ((value & _currentRawPadState) && !(value & _lastRawPadState)) {
            return value;
        }
    }
    return -1;
}

void Input::SaveInputTable()
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, kKeyConfigFilename.c_str(), "wb");  //バイナリで「書き込み」
    //識別子書き込み(４バイト)
    std::string signature = kKeyConfigSignature;                //ファイルを識別するための識別子
    fwrite(signature.data(), signature.size(), 1, fp);          //識別子の書き込み

    //バージョンの書き込み
    const float version = 1.0f;
    fwrite(&version, sizeof(version), 1, fp);//4バイト

    //データサイズの書き込み
    auto size = _inputTable.size();
    fwrite(&size, sizeof(size), 1, fp);//4バイト

    //データ本体の書き込み
    for (const auto& record : _inputTable) {
        //キーの書き込み(イベント名)
        byte nameSize = static_cast<byte>(record.first.length());//文字列数
        fwrite(&nameSize, sizeof(nameSize), 1, fp);//文字列のサイズ
        fwrite(record.first.data(), record.first.size(), 1, fp);//文字列本体
        //実データ値の書き込み(「入力種別＋入力ID」の配列)
        //まずは、実データ配列のサイズを取得します
        byte inputDataSize = static_cast<byte>(record.second.size());
        fwrite(&inputDataSize, sizeof(inputDataSize), 1, fp);
        fwrite(record.second.data(), //書き込む実データの先頭アドレス
            sizeof(record.second[0]), //書き込むデータ１つあたりのサイズ
            record.second.size(), //書き込むデータ数
            fp);
    }

    fclose(fp);
}

void Input::LoadInputTable()
{
    std::string filename = kKeyConfigFilename;
    auto wfilename = StringUtility::GetWStringFromString(filename);
    auto handle = FileRead_open(wfilename.c_str());
    if (handle == 0) {//何頭の原因で読み込み失敗したら読み込まない
        return;
    }
    struct Header {
        char signature[4];//シグネチャ
        float version;//バージョン
        size_t dataNum;//データ数
    };
    Header header = {};//ヘッダーの読み込み
    FileRead_read(&header, sizeof(header), handle);
    //データ数の分だけ読み込んでいく
    for (int i = 0; i < header.dataNum; ++i) {
        //データはまず、イベント名を読み込む
        //名前のサイズ
        byte nameSize = 0;
        FileRead_read(&nameSize, sizeof(nameSize), handle);
        //実際に名前文字列を取得
        std::string strEventName;
        strEventName.resize(nameSize);//名前のサイズ分確保
        FileRead_read(strEventName.data(), nameSize * sizeof(char), handle);

        //名前が終わった後は実データなので、まず実データ個数を取得
        byte inputDataNum = 0;
        FileRead_read(&inputDataNum, sizeof(inputDataNum), handle);
        std::vector<InputState> inputStates;
        inputStates.resize(inputDataNum);
        FileRead_read(inputStates.data(), sizeof(inputStates[0]) * inputDataNum, handle);
        _inputTable[strEventName] = inputStates;
    }

    FileRead_close(handle);
}
