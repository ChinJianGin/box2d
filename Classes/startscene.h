#pragma once
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "common/CButton.h"
#include "StaticDynamicScene.h"
#include "FixtureCollisionScene.h"
#include "stageone.h"
#include "stagetwo.h"
#include "stagethree.h"
#include "stagefour.h"

#define STAGE_NUM 4

class StartScene : public cocos2d::Scene
{
public:

    StartScene();
    ~StartScene();

    static cocos2d::Scene* createScene();
    virtual bool init();
    void update(float dt);

    bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件,回傳值必須是 bool
    void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
    void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

    // implement the "static create()" method manually
    CREATE_FUNC(StartScene); //展開後定義了 create() 成員函式

private:
    CButton* _startBtn[STAGE_NUM];
    bool _bToGameScene;
    int _stage;

};