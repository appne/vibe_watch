#include <pebble.h>

/*
  vibe watch --- 2015/02/25 appne
  振動で時刻を通知する。0:00～11:59分の12時間制。
  読み方は、短い振動が前半、長い振動で後半を原則として、アナログ時計の文字盤を分割していくイメージ。
  1秒間のブランクをセパレータに、[時][分][分詳細]を振動する。
  例外1)正時は、[時]以外振動しない。
  例外2)[時]もしくは[分]で、短短と長短のパターン(文字盤の12と6位置)は、短もしくは長の1回だけ振動する。
  
  [時]　0時を基準として
  1回目の振動:短=+0時間、長=+6時間
  2回目の振動：短=+0時間、長=+3時間
  3回目の振動：なし=+0時間、短=+1時間、長=+2時間
  例)　短で0時、短短短で1時、長で6時、長長長で11時
  
  [分]　0分を基準として
  1回目の振動:短=+0分、長=+30分
  2回目の振動：短=+0分、長=+15分
  3回目の振動：なし=+0分、短=+5分、長=+10分
  例)　短で0分、短短短で5分、長で30分、長長長で55分
  
  [分詳細]　5分単位未満の細かい分情報
  +0分：振動無し
  +1分：短
  +2分：短短
  +3分：長
  +4分：長短
  
  例）
  1:54 短短短　長長短　長短
*/

// 振動時間定数
#define No_Vibe 0
#define Short_Vibe 50
#define Long_Vibe 200
#define Short_Blank 400
#define Long_Blank 1000

//　vibe制御構造体
static uint32_t segments[17] = {0};
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};

// 文字盤の12分割情報設定
int set12Vibe(int index, int num)
{
  num %= 12;
  segments[index++] = (num < 6)? Short_Vibe: Long_Vibe;
  segments[index++] = Short_Blank;
  num %=6;
  if (num) { // 文字盤の12ならびに6位置では余分に振動しない
    segments[index++] = (num < 3)? Short_Vibe: Long_Vibe;
    segments[index++] = Short_Blank;
  }
  num %=3;
  switch (num) {
  case 0:    // nothing
    segments[index++] = No_Vibe;
    break;
  case 1:    // one short
    segments[index++] = Short_Vibe;
    break;
  case 2:    // one long
    segments[index++] = Long_Vibe;
    break;
  }
  segments[index++] = Long_Blank;
  return index;
}

int main(void) {
  time_t now;
  struct tm* pNow;
  int index = 0;

  // initial blank
  segments[index++] = No_Vibe;
  segments[index++] = Short_Blank;
    
  time(&now);
  pNow = localtime(&now);
  //　時　設定
  index = set12Vibe(index, pNow->tm_hour);
  
  if (pNow->tm_min) { // 正時には振動しない
    //　5分単位　設定
    index = set12Vibe(index, pNow->tm_min / 5);

    //　分詳細　設定
    switch (pNow->tm_min % 5) {
    case 0:
      break;
    case 2:
      segments[index++] = Short_Vibe;
      segments[index++] = Short_Blank;
      /* THROUGH */
    case 1:
      segments[index++] = Short_Vibe;
      break;
    case 3:
      segments[index++] = Long_Vibe;
      break;
    case 4:
      segments[index++] = Long_Vibe;
      segments[index++] = Short_Blank;
      segments[index++] = Short_Vibe;
      break;
    }
  }

  vibes_enqueue_custom_pattern(pat);
  {
      int sum = 0;
      while (--index >= 0) {
          sum += segments[index];
      }
      sum += Long_Blank;
      psleep(sum);        // wait for end vibe
  }
}
