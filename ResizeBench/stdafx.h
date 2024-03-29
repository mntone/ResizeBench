// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル
#include <windows.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>

// C++ ランタイム ヘッダー ファイル
#include <memory>

// D2D
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#include <MMSystem.h>

#include "Bench.h"
