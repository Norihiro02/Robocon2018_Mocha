/*
        @file   loop_cycle.h
        @brief  ループ周期の純粋仮想クラス
        @author 谷川　豊章
        @date   2016/9/1
*/

#pragma once

//  ループ周期の純粋仮想クラス

class LoopCycle {

    const unsigned long CYCLE_US;   ///< 周期(unit:us)

  public:

    /**     コンストラクタ
            @param  cycle_us    [in]目標のループ周期
    */
    LoopCycle(unsigned long cycle_us) : CYCLE_US(cycle_us)
    { }

    /**     デストラクタ
    */
    virtual ~LoopCycle(){}

    /**     ループ周期の読み出し
            @return ループ周期
    */
    unsigned long getCycleUs() {
      return CYCLE_US;
    }

};
